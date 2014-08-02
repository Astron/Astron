#include <iomanip>

// These are helpers for decoding MessagePack data:
template<typename T>
static inline T msgpack_swap(T value)
{
    if(sizeof(value) == 1) {
        return value;
    } else if(sizeof(value) == 2) {
        uint16_t *x = (uint16_t*)&value;
        uint16_t y;
        y = (*x & 0x00ff) << 8 |
            (*x & 0xff00) >> 8;
        return ((T)y);
    } else if(sizeof(value) == 4) {
        uint32_t *x = (uint32_t*)&value;
        uint32_t y;
        y = (*x & 0x000000ff) << 24 |
            (*x & 0x0000ff00) <<  8 |
            (*x & 0x00ff0000) >>  8 |
            (*x & 0xff000000) >> 24;
        return ((T)y);
    } else if(sizeof(value) == 8) {
        uint64_t *x = (uint64_t*)&value;
        uint64_t y;
        y = (*x & 0x00000000000000ff) << 56 |
            (*x & 0x000000000000ff00) << 40 |
            (*x & 0x0000000000ff0000) << 24 |
            (*x & 0x00000000ff000000) <<  8 |
            (*x & 0x000000ff00000000) >>  8 |
            (*x & 0x0000ff0000000000) >> 24 |
            (*x & 0x00ff000000000000) >> 40 |
            (*x & 0xff00000000000000) >> 56;
        return ((T)y);
    }
}

static void msgpack_decode(std::ostream &out, DatagramIterator &dgi);

static void msgpack_decode_container(std::ostream &out, DatagramIterator &dgi, int length, bool map)
{
    if(map) {
        out << "{";
    } else {
        out << "[";
    }

    for(int i = 0; i < length; ++i) {
        if(i != 0) {
            out << ", ";
        }

        if(map) {
            msgpack_decode(out, dgi);
            out << ": ";
        }

        msgpack_decode(out, dgi);
    }

    if(map) {
        out << "}";
    } else {
        out << "]";
    }
}

static char json_escapes[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 'b', 't', 'n', 'v', 'f', 'r', 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static void msgpack_decode_string(std::ostream &out, DatagramIterator &dgi, int length)
{
    out << '"';

    for(int i = 0; i < length; ++i) {
        unsigned char output = dgi.read_uint8();
        if(output < 0x20) {
            if(json_escapes[output]) {
                out << '\\' << json_escapes[output];
                continue;
            }
        } else if(output == '"') {
            out << "\\\"";
            continue;
        } else if(output == '\\') {
            out << "\\\\";
            continue;
        } else if(output < 0x7F) {
            out << output;
            continue;
        }

        // If we got here, we have to escape it:
        out << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)output;
    }

    out << '"';
}

static void msgpack_decode_ext(std::ostream &out, DatagramIterator &dgi, int length)
{
    out << "ext(" << dgi.read_uint8() << ", ";
    msgpack_decode_string(out, dgi, length);
    out << ")";
}

static void msgpack_decode(std::ostream &out, DatagramIterator &dgi)
{
    uint8_t msg = dgi.read_uint8();
    if(msg < 0x80) {
        // fixint8
        out << (int)msg;
    } else if(msg <= 0x8f) {
        // fixmap
        msgpack_decode_container(out, dgi, msg - 0x80, true);
    } else if(msg <= 0x9f) {
        // fixarray
        msgpack_decode_container(out, dgi, msg - 0x90, false);
    } else if(msg <= 0xbf) {
        // fixstr
        msgpack_decode_string(out, dgi, msg - 0xa0);
    } else if(msg == 0xc0) {
        out << "null";
    } else if(msg == 0xc1) {
        out << "*INVALID*";
    } else if(msg == 0xc2) {
        out << "false";
    } else if(msg == 0xc3) {
        out << "true";
    } else if(msg == 0xc4) {
        // bin8
        msgpack_decode_string(out, dgi, dgi.read_uint8());
    } else if(msg == 0xc5) {
        // bin16
        msgpack_decode_string(out, dgi, msgpack_swap(dgi.read_uint16()));
    } else if(msg == 0xc6) {
        // bin32
        msgpack_decode_string(out, dgi, msgpack_swap(dgi.read_uint32()));
    } else if(msg == 0xc7) {
        // ext8
        msgpack_decode_ext(out, dgi, dgi.read_uint8());
    } else if(msg == 0xc8) {
        // ext16
        msgpack_decode_ext(out, dgi, msgpack_swap(dgi.read_uint16()));
    } else if(msg == 0xc9) {
        // ext32
        msgpack_decode_ext(out, dgi, msgpack_swap(dgi.read_uint32()));
    } else if(msg == 0xca) {
        // float32
        out << msgpack_swap(dgi.read_float32());
    } else if(msg == 0xcb) {
        // float64
        out << msgpack_swap(dgi.read_float64());
    } else if(msg == 0xcc) {
        // uint8
        out << dgi.read_uint8();
    } else if(msg == 0xcd) {
        // uint16
        out << msgpack_swap(dgi.read_uint16());
    } else if(msg == 0xce) {
        // uint32
        out << msgpack_swap(dgi.read_uint32());
    } else if(msg == 0xcf) {
        // uint64
        out << msgpack_swap(dgi.read_uint64());
    } else if(msg == 0xd0) {
        // int8
        out << dgi.read_int8();
    } else if(msg == 0xd1) {
        // int16
        out << msgpack_swap(dgi.read_int16());
    } else if(msg == 0xd2) {
        // int32
        out << msgpack_swap(dgi.read_int32());
    } else if(msg == 0xd3) {
        // int64
        out << msgpack_swap(dgi.read_int64());
    } else if(msg <= 0xd8) {
        // fixext
        msgpack_decode_ext(out, dgi, 1 << (msg - 0xd4));
    } else if(msg == 0xd9) {
        // str8
        msgpack_decode_string(out, dgi, dgi.read_uint8());
    } else if(msg == 0xda) {
        // str16
        msgpack_decode_string(out, dgi, msgpack_swap(dgi.read_uint16()));
    } else if(msg == 0xdb) {
        // str32
        msgpack_decode_string(out, dgi, msgpack_swap(dgi.read_uint32()));
    } else if(msg == 0xdc) {
        // array16
        msgpack_decode_container(out, dgi, msgpack_swap(dgi.read_uint16()), false);
    } else if(msg == 0xdd) {
        // array32
        msgpack_decode_container(out, dgi, msgpack_swap(dgi.read_uint32()), false);
    } else if(msg == 0xde) {
        // map16
        msgpack_decode_container(out, dgi, msgpack_swap(dgi.read_uint16()), true);
    } else if(msg == 0xdf) {
        // map32
        msgpack_decode_container(out, dgi, msgpack_swap(dgi.read_uint32()), true);
    } else {
        // Everything >=0xe0 is a negative fixint.
        int8_t value = (int8_t)msg;
        out << (int)value;
    }
}
