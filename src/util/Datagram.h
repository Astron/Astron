#pragma once
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <string.h> // memcpy
#include <memory>
#include "core/types.h"
#include "dclass/util/byteorder.h"

#ifdef ASTRON_32BIT_DATAGRAMS
typedef uint32_t dgsize_t;
#else
typedef uint16_t dgsize_t;
#endif

#define DGSIZE_MAX ((dgsize_t)(-1))


class Datagram; // foward declaration
typedef std::shared_ptr<Datagram> DatagramPtr;
typedef std::shared_ptr<const Datagram> DatagramHandle;

// A DatagramOverflow is an exception which occurs when an add_<value> method is called which would
// increase the size of the datagram past DGSIZE_MAX (preventing integer and buffer overflow).
class DatagramOverflow : public std::runtime_error
{
  public:
    DatagramOverflow(const std::string &what) : std::runtime_error(what) { }
};

// A Datagram is a buffer of binary data ready for networking (ie. formatted according to Astron's
// over-the-wire formatting specification).  It is most often used to represent Astron client and
// server messages, as well as occasionally DistributedObject field data.
class Datagram
{
  protected:
    uint8_t* buf;
    dgsize_t buf_cap;
    dgsize_t buf_offset;

    void check_add_length(dgsize_t len)
    {
        if(buf_offset + len > DGSIZE_MAX) {
            std::stringstream err_str;
            err_str << "dg tried to add data past max datagram size, buf_offset+len("
                    << buf_offset + len << ")" << " max_size(" << DGSIZE_MAX << ")" << std::endl;
            throw DatagramOverflow(err_str.str());
        }

        if(buf_offset + len > buf_cap) {
            uint8_t *tmp_buf = new uint8_t[buf_cap + len + 64];
            memcpy(tmp_buf, buf, buf_cap);
            delete [] buf;
            buf = tmp_buf;
            buf_cap = buf_cap + len + 64;
        }
    }
    // default-constructor:
    //     creates a new datagram with some pre-allocated space
    Datagram() : buf(new uint8_t[64]), buf_cap(64), buf_offset(0)
    {
    }

    /*
    // TODO: Deal with overload collision with Datagram(uint16_t message_type)

    // sized-constructor:
    //     allows you to specify the capacity of the datagram ahead of time,
    //     this should be used when the exact size is known ahead of time for performance
    Datagram(dgsize_t capacity) : buf(new uint8_t[capacity]), buf_cap(capacity),
    	buf_offset(0)
    {
    }
    */

    // copy-constructor:
    //     creates a new datagram which is a deep-copy of another datagram;
    //     capacity is not perserved and instead is reduced to the size of the source datagram.
    Datagram(const Datagram &dg) : buf(new uint8_t[dg.size()]), buf_cap(dg.size()),
        buf_offset(dg.size())
    {
        memcpy(buf, dg.buf, dg.size());
    }

    // shallow-constructor:
    //     creates a new datagram that uses an existing buffer as its data
    Datagram(uint8_t *data, dgsize_t length, dgsize_t capacity) : buf(data),
        buf_cap(capacity), buf_offset(length)
    {
    }

    // binary-constructor(pointer):
    //     creates a new datagram with a copy of the data contained at the pointer.
    Datagram(const uint8_t *data, dgsize_t length) : buf(new uint8_t[length]), buf_cap(length),
        buf_offset(length)
    {
        memcpy(buf, data, length);
    }

    // binary-constructor(vector):
    //     creates a new datagram with a copy of the binary data contained in a vector<uint8_t>.
    Datagram(const std::vector<uint8_t> &data) : buf(new uint8_t[data.size()]),
        buf_cap(data.size()), buf_offset(data.size())
    {
        memcpy(buf, &data[0], data.size());
    }

    // binary-constructor(string):
    //     creates a new datagram with a copy of the data contained in a string, treated as binary.
    Datagram(const std::string &data) : buf(new uint8_t[data.length()]), buf_cap(data.length()),
        buf_offset(data.length())
    {
        memcpy(buf, data.c_str(), data.length());
    }

    // server-header-constructor(single-receiver):
    //     creates a new datagram initialized with a server header (accepts only 1 receiver).
    Datagram(channel_t to_channel, channel_t from_channel, uint16_t message_type) :
        buf(new uint8_t[64]), buf_cap(64), buf_offset(0)
    {
        add_server_header(to_channel, from_channel, message_type);
    }

    // server-header-constructor(multi-target):
    //     creates a new datagram initialized with a server header (accepts a set of receivers)
    Datagram(const std::set<channel_t> &to_channels, channel_t from_channel,
             uint16_t message_type) : buf(new uint8_t[64]), buf_cap(64), buf_offset(0)
    {
        add_server_header(to_channels, from_channel, message_type);
    }

    // control-header constructor:
    //     creates a new datagram initialized with a control header containing the msgtype.
    Datagram(uint16_t message_type) : buf(new uint8_t[64]), buf_cap(64), buf_offset(0)
    {
        add_control_header(message_type);
    }


  public:
    static DatagramPtr create()
    {
        DatagramPtr dg_ptr(new Datagram);
        return dg_ptr;
    }

    static DatagramPtr create(DatagramHandle dg)
    {
        DatagramPtr dg_ptr(new Datagram(*dg.get()));
        return dg_ptr;
    }

    static DatagramPtr create(uint8_t *data, dgsize_t length, dgsize_t capacity)
    {
        DatagramPtr dg_ptr(new Datagram(data, length, capacity));
        return dg_ptr;
    }

    static DatagramPtr create(const uint8_t *data, dgsize_t length)
    {
        DatagramPtr dg_ptr(new Datagram(data, length));
        return dg_ptr;
    }

    static DatagramPtr create(const std::vector<uint8_t> &data)
    {
        DatagramPtr dg_ptr(new Datagram(data));
        return dg_ptr;
    }

    static DatagramPtr create(const std::string &data)
    {
        DatagramPtr dg_ptr(new Datagram(data));
        return dg_ptr;
    }

    static DatagramPtr create(channel_t to_channel, channel_t from_channel,
                              uint16_t message_type)
    {
        DatagramPtr dg_ptr(new Datagram(to_channel, from_channel, message_type));
        return dg_ptr;
    }

    static DatagramPtr create(const std::set<channel_t> &to_channels,
                              channel_t from_channel,
                              uint16_t message_type)
    {
        DatagramPtr dg_ptr(new Datagram(to_channels, from_channel, message_type));
        return dg_ptr;
    }

    static DatagramPtr create(uint16_t message_type)
    {
        DatagramPtr dg_ptr(new Datagram(message_type));
        return dg_ptr;
    }


    // destructor
    ~Datagram()
    {
        delete [] buf;
    }

    // add_bool adds an 8-bit integer to the datagram that is guaranteed
    // to be one of the values 0x00 (false) or 0x01 (true).
    void add_bool(const bool &v)
    {
        if(v) add_uint8(1);
        else add_uint8(0);
    }

    // add_int8 adds a signed 8-bit integer value to the datagram.
    void add_int8(const int8_t &v)
    {
        check_add_length(1);
        *(int8_t *)(buf + buf_offset) = v;
        buf_offset += 1;
    }

    // add_int16 adds a signed 16-bit integer value to the datagram arranged in little-endian.
    void add_int16(const int16_t &v)
    {
        check_add_length(2);
        *(int16_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += 2;
    }

    // add_int32 adds a signed 32-bit integer value to the datagram arranged in little-endian.
    void add_int32(const int32_t &v)
    {
        check_add_length(4);
        *(int32_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += 4;
    }

    // add_int64 adds a signed 64-bit integer value to the datagram arranged in little-endian.
    void add_int64(const int64_t &v)
    {
        check_add_length(8);
        *(int64_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += 8;
    }

    // add_uint8 adds an unsigned 8-bit integer value to the datagram.
    void add_uint8(const uint8_t &v)
    {
        check_add_length(1);
        *(uint8_t *)(buf + buf_offset) = v;
        buf_offset += 1;
    }

    // add_uint16 adds a unsigned 16-bit integer value to the datagram arranged in little-endian.
    void add_uint16(const uint16_t &v)
    {
        check_add_length(2);
        *(uint16_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += 2;
    }

    // add_uint32 adds a unsigned 32-bit integer value to the datagram arranged in little-endian.
    void add_uint32(const uint32_t &v)
    {
        check_add_length(4);
        *(uint32_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += 4;
    }

    // add_uint64 adds a unsigned 64-bit integer value to the datagram arranged in little-endian.
    void add_uint64(const uint64_t &v)
    {
        check_add_length(8);
        *(uint64_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += 8;
    }

    // add_float32 adds a float (32-bit IEEE 754 floating point) value to the datagram.
    void add_float32(const float &v)
    {
        check_add_length(4);
        *(float *)(buf + buf_offset) = swap_le(v);
        buf_offset += 4;
    }

    // add_float64 adds a double (64-bit IEEE 754 floating point) value to the datagram.
    void add_float64(const double &v)
    {
        check_add_length(8);
        *(double *)(buf + buf_offset) = swap_le(v);
        buf_offset += 8;
    }

    // add_size adds a datagram or field length-tag to the datagram.
    // Note: this method should always be used instead of add_uint16 when adding a length tag
    //       to allow for future support of larger or small length limits.
    void add_size(const dgsize_t &v)
    {
        check_add_length(sizeof(dgsize_t));
        *(dgsize_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += sizeof(dgsize_t);
    }

    // add_channel adds a channel to the end of the datagram.
    // Note: this method should always be used instead of add_uint64 when adding a channel
    //       to allow for future support of larger or smaller channel range limits.
    void add_channel(const channel_t &v)
    {
        check_add_length(sizeof(channel_t));
        *(channel_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += sizeof(channel_t);
    }

    // add_doid adds a distributed object id to the end of the datagram.
    // Note: this method should always be used instead of add_uint32 when adding a doid
    //       to allow for future support of larger or smaller doid range limits.
    void add_doid(const doid_t &v)
    {
        check_add_length(sizeof(doid_t));
        *(doid_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += sizeof(doid_t);
    }

    // add_zone adds a zone id to the end of the datagram.
    // Note: this method should always be used instead of add_uint32 when adding a zone
    //       to allow for future support of larger or smaller zone range limits.
    void add_zone(const zone_t &v)
    {
        check_add_length(sizeof(zone_t));
        *(zone_t *)(buf + buf_offset) = swap_le(v);
        buf_offset += sizeof(zone_t);
    }

    // add_location adds a parent-zone pair to the datagram; it is provided for convenience,
    // as well as code legibility and is slightly more performant than adding the parent
    // and zone separately.
    void add_location(const doid_t &parent, const zone_t &zone)
    {
        check_add_length(sizeof(doid_t) + sizeof(zone_t));
        *(doid_t *)(buf + buf_offset) = swap_le(parent);
        buf_offset += sizeof(doid_t);
        *(zone_t *)(buf + buf_offset) = swap_le(zone);
        buf_offset += sizeof(zone_t);
    }

    // add_data adds raw binary data directly to the end of the datagram.
    void add_data(const std::vector<uint8_t> &data)
    {
        if(data.size()) {
            check_add_length(data.size());
            memcpy(buf + buf_offset, &data[0], data.size());
            buf_offset += data.size();
        }
    }
    void add_data(const std::string &str)
    {
        if(str.length()) {
            check_add_length(str.length());
            memcpy(buf + buf_offset, str.c_str(), str.length());
            buf_offset += str.length();
        }
    }
    void add_data(const uint8_t* data, dgsize_t length)
    {
        if(length) {
            check_add_length(length);
            memcpy(buf + buf_offset, data, length);
            buf_offset += length;
        }
    }
    void add_data(DatagramHandle dg)
    {
        if(dg->buf_offset) {
            check_add_length(dg->buf_offset);
            memcpy(buf + buf_offset, dg->buf, dg->buf_offset);
            buf_offset += dg->buf_offset;
        }
    }

    // add_string adds a dclass string to the datagram from binary data;
    // a length tag (typically a uint16_t) is prepended to the string before it is added.
    void add_string(const std::string &str)
    {
        add_size(str.length());
        check_add_length(str.length());
        memcpy(buf + buf_offset, str.c_str(), str.length());
        buf_offset += str.length();
    }
    void add_string(const char* str, dgsize_t length)
    {
        add_size(length);
        check_add_length(length);
        memcpy(buf + buf_offset, str, length);
        buf_offset += length;
    }

    // add_blob adds a dclass blob to the datagram from binary data;
    // a length tag (typically a uint16_t) is prepended to the blob before it is added.
    void add_blob(const std::vector<uint8_t> &blob)
    {
        add_size(blob.size());
        check_add_length(blob.size());
        memcpy(buf + buf_offset, &blob[0], blob.size());
        buf_offset += blob.size();
    }
    void add_blob(const uint8_t* data, dgsize_t length)
    {
        add_size(length);
        check_add_length(length);
        memcpy(buf + buf_offset, data, length);
        buf_offset += length;
    }
    void add_blob(DatagramHandle dg)
    {
        add_size(dg->buf_offset);
        check_add_length(dg->buf_offset);
        memcpy(buf + buf_offset, dg->buf, dg->buf_offset);
        buf_offset += dg->buf_offset;
    }

    // add_buffer reserves a buffer of size "length" at the end of the datagram
    // and returns a pointer to the buffer so it can be filled manually
    uint8_t* add_buffer(dgsize_t length)
    {
        check_add_length(length);
        uint8_t* buf_start = buf + buf_offset;
        buf_offset += length;
        return buf_start;
    }

    // add_server_header prepends a generic header for messages that are supposed to be routed
    // to one or more role instances within the server cluster. The method is provided entirely
    // for convenience.
    //
    // The format of a server header is:
    //     (uint8_t num_targets, channel_t[] targets, channel_t sender, uint16_t message_type)
    // Note that other types of datagrams do not always include a sender; therefore, the sender
    // and message_type are actually considered part of the payload of the datagram.
    void add_server_header(channel_t to, channel_t from, uint16_t message_type)
    {
        add_uint8(1);
        add_channel(to);
        add_channel(from);
        add_uint16(message_type);
    }
    void add_server_header(const std::set<channel_t> &to, channel_t from, uint16_t message_type)
    {
        add_uint8(to.size());
        for(auto it = to.begin(); it != to.end(); ++it) {
            add_channel(*it);
        }
        add_channel(from);
        add_uint16(message_type);
    }

    // add_control_header prepends a header for control messages that are handled by a
    // MessageDirector instance. The method is provided entirely for convenience.
    //
    // The format of a control header is:
    //     (uint8_t 1, channel_t CONTROL_MESSAGE, uint16_t message type)
    // Note that other types of datagrams optionally include a sender before the message_type;
    // therefore, the message_type is actually considered part of the payload of the datagram.
    void add_control_header(uint16_t message_type)
    {
        add_uint8(1);
        add_channel(CONTROL_MESSAGE);
        add_uint16(message_type);
    }

    // size returns the amount of data added to the datagram in bytes.
    dgsize_t size() const
    {
        return buf_offset;
    }

    // cap returns the currently allocated size of the datagram in memory (ie. capacity).
    // Note: the datagram handles resizing automatically so this method is primarily available
    //       for debugging, and possible performance considerations.
    dgsize_t cap() const
    {
        return buf_cap;
    }

    // get_data returns a pointer to the start of the Datagram's data buffer.
    const uint8_t* get_data() const
    {
        return buf;
    }
};
