#pragma once
#include "Datagram.h"
#include "dclass/dc/Struct.h"
#include "dclass/dc/Method.h"
#include "dclass/dc/Field.h"
#include "dclass/dc/Parameter.h"
#ifdef _DEBUG
#include <fstream>
#endif

// A DatagramIteratorEOF is an exception that is thrown when attempting to read
// past the end of a datagram.
class DatagramIteratorEOF : public std::runtime_error
{
  public:
    DatagramIteratorEOF(const std::string &what) : std::runtime_error(what) { }
};

// A DatagramIterator lets you step trough a datagram by reading a single value at a time.
class DatagramIterator
{
  protected:
    DatagramHandle m_dg;
    dgsize_t m_offset;

    void check_read_length(dgsize_t length)
    {
        if(m_offset + length > m_dg->size()) {
            std::stringstream error;
            error << "dgi tried to read past dg end, offset+length(" << m_offset + length << ")"
                  << " buf_size(" << m_dg->size() << ")" << std::endl;
            throw DatagramIteratorEOF(error.str());
        };
    }
  public:
    // constructor
    DatagramIterator(DatagramHandle dg, dgsize_t offset = 0) : m_dg(dg), m_offset(offset)
    {
        check_read_length(0); //shortcuts, yay
    }

    // read_bool reads the next byte from the datagram and returns either false or true.
    bool read_bool()
    {
        uint8_t val = read_uint8();
        return val != false; // returns either 1 or 0
    }

    // read_int8 reads a byte from the datagram,
    // returning a signed 8-bit integer.
    int8_t read_int8()
    {
        check_read_length(1);
        int8_t r = *(int8_t*)(m_dg->get_data() + m_offset);
        m_offset += 1;
        return r;
    }

    // read_int16 reads 2 bytes from the datagram,
    // returning a signed 16-bit integer in native endianness.
    int16_t read_int16()
    {
        check_read_length(2);
        int16_t r = *(int16_t*)(m_dg->get_data() + m_offset);
        m_offset += 2;
        return swap_le(r);
    }

    // read_int32 reads 4 bytes from the datagram,
    // returning a signed 32-bit integer in native endianness.
    int32_t read_int32()
    {
        check_read_length(4);
        int32_t r = *(int32_t*)(m_dg->get_data() + m_offset);
        m_offset += 4;
        return swap_le(r);
    }

    // read_int64 reads 8 bytes from the datagram,
    // returning a signed 64-bit integer in native endianness.
    int64_t read_int64()
    {
        check_read_length(8);
        int64_t r = *(int64_t*)(m_dg->get_data() + m_offset);
        m_offset += 8;
        return swap_le(r);
    }

    // read_uint8 reads a byte from the datagram,
    // returning an unsigned 8-bit integer.
    uint8_t read_uint8()
    {
        check_read_length(1);
        uint8_t r = *(uint8_t*)(m_dg->get_data() + m_offset);
        m_offset += 1;
        return r;
    }

    // read_uint16 reads 2 bytes from the datagram,
    // returning an unsigned 16-bit integer in native endianness.
    uint16_t read_uint16()
    {
        check_read_length(2);
        uint16_t r = *(uint16_t*)(m_dg->get_data() + m_offset);
        m_offset += 2;
        return swap_le(r);
    }

    // read_uint32 reads 4 bytes from the datagram,
    // returning an unsigned 32-bit integer in native endianness.
    uint32_t read_uint32()
    {
        check_read_length(4);
        uint32_t r = *(uint32_t*)(m_dg->get_data() + m_offset);
        m_offset += 4;
        return swap_le(r);
    }

    // read_uint64 reads 8 bytes from the datagram,
    // returning an unsigned 64-bit integer in native endianness.
    uint64_t read_uint64()
    {
        check_read_length(8);
        uint64_t r = *(uint64_t*)(m_dg->get_data() + m_offset);
        m_offset += 8;
        return swap_le(r);
    }

    // read_size reads a dgsize_t from the datagram.
    dgsize_t read_size()
    {
        check_read_length(sizeof(dgsize_t));
        dgsize_t r = *(dgsize_t*)(m_dg->get_data() + m_offset);
        m_offset += sizeof(dgsize_t);
        return swap_le(r);
    }

    // read_channel reads a channel_t from the datagram.
    channel_t read_channel()
    {
        check_read_length(sizeof(channel_t));
        channel_t r = *(channel_t*)(m_dg->get_data() + m_offset);
        m_offset += sizeof(channel_t);
        return swap_le(r);
    }

    // read_doid reads a doid_t from the datagram.
    doid_t read_doid()
    {
        check_read_length(sizeof(doid_t));
        doid_t r = *(doid_t*)(m_dg->get_data() + m_offset);
        m_offset += sizeof(doid_t);
        return swap_le(r);
    }

    // read_zone reads a zone_t from the datagram.
    zone_t read_zone()
    {
        check_read_length(sizeof(zone_t));
        zone_t r = *(zone_t*)(m_dg->get_data() + m_offset);
        m_offset += sizeof(zone_t);
        return swap_le(r);
    }

    // read_float32 reads 4 bytes from the datagram,
    // returning a 32-bit float in native endianness.
    float read_float32()
    {
        check_read_length(4);
        float r = *(float*)(m_dg->get_data() + m_offset);
        m_offset += 4;
        return swap_le(r);
    }

    // read_float64 reads 8 bytes from the datagram,
    // returning a 64-bit float (double) in native endianness.
    double read_float64()
    {
        check_read_length(8);
        double r = *(double*)(m_dg->get_data() + m_offset);
        m_offset += 8;
        return swap_le(r);
    }

    // read_string reads a string from the datagram in the format
    //     {dgsize_t length; char[length] characters} and returns the character data.
    std::string read_string()
    {
        dgsize_t length = read_size();
        check_read_length(length);
        std::string str((char*)(m_dg->get_data() + m_offset), length);
        m_offset += length;
        return str;
    }

    // read_blob reads a blob from the datagram in the format
    //     {dgsize_t length; uint8[length] binary} and returns the binary part.
    std::vector<uint8_t> read_blob()
    {
        dgsize_t length = read_size();
        return read_data(length);
    }

    // read_datagram reads a blob from the datagram and returns it as another datagram.
    DatagramPtr read_datagram()
    {
        dgsize_t length = read_size();
        return Datagram::create(m_dg->get_data() + m_offset, length);
    }

    // read_data returns the next <length> bytes in the datagram.
    std::vector<uint8_t> read_data(dgsize_t length)
    {
        check_read_length(length);
        std::vector<uint8_t> data(m_dg->get_data() + m_offset, m_dg->get_data() + m_offset + length);
        m_offset += length;
        return data;
    }

    // read_remainder returns a vector containing the rest of the bytes in the datagram.
    std::vector<uint8_t> read_remainder()
    {
        return read_data(m_dg->size() - m_offset);
    }


    // unpack_field accepts a Field of a distributed class
    //     and returns the packed value for the field.
    std::vector<uint8_t> unpack_field(const dclass::Field* field)
    {
        std::vector<uint8_t> buffer;
        unpack_field(field, buffer);
        return buffer;
    }

    // unpack_field can also be called to read into an existing buffer.
    void unpack_field(const dclass::Field* field, std::vector<uint8_t> &buffer)
    {
        unpack_dtype(field->get_type(), buffer);
    }

    // unpack_dtype accepts a DistributedType and copies the data for the value into a buffer.
    void unpack_dtype(const dclass::DistributedType* dtype, std::vector<uint8_t> &buffer)
    {
        using namespace dclass;
        if(dtype->has_fixed_size()) {
            // If field is a fixed-sized type like uint, int, float, etc
            // Also any other type lucky enough to be fixed size will be computed faster
            std::vector<uint8_t> data = read_data(dtype->get_size());
            buffer.insert(buffer.end(), data.begin(), data.end());
            return;
        }

        // For the unlucky types, we have to figure out their size manually
        switch(dtype->get_type()) {
        case T_VARSTRING:
        case T_VARBLOB:
        case T_VARARRAY: {
            dgsize_t len = read_size();
            dgsize_t net_len = swap_le(len);
            buffer.insert(buffer.end(), (uint8_t*)&net_len, (uint8_t*)&net_len + sizeof(dgsize_t));

            std::vector<uint8_t> blob = read_data(len);
            buffer.insert(buffer.end(), blob.begin(), blob.end());
            break;
        }
        case T_STRUCT: {
            const Struct* dstruct = dtype->as_struct();
            size_t num_fields = dstruct->get_num_fields();
            for(unsigned int i = 0; i < num_fields; ++i) {
                unpack_dtype(dstruct->get_field(i)->get_type(), buffer);
            }
            break;
        }
        case T_METHOD: {
            const Method* dmethod = dtype->as_method();
            size_t num_params = dmethod->get_num_parameters();
            for(unsigned int i = 0; i < num_params; ++i) {
                unpack_dtype(dmethod->get_parameter(i)->get_type(), buffer);
            }
            break;
        }
        default: {
            // This case should be impossible, but a default is required by compilers
            break;
        }
        }
    }

    // skip_field can be used to seek past the packed field data for a Field.
    //     Throws DatagramIteratorEOF if it skips past the end of the datagram.
    void skip_field(const dclass::Field* field)
    {
        skip_dtype(field->get_type());
    }

    // skip_dtype can be used to seek past the packed data for a DistributedType.
    //     Throws DatagramIteratorEOF if it skips past the end of the datagram.
    void skip_dtype(const dclass::DistributedType *dtype)
    {
        using namespace dclass;
        if(dtype->has_fixed_size()) {
            dgsize_t length = dtype->get_size();
            check_read_length(length);
            m_offset += length;
            return;
        }

        switch(dtype->get_type()) {
        case T_VARSTRING:
        case T_VARBLOB:
        case T_VARARRAY: {
            dgsize_t length = read_size();
            check_read_length(length);
            m_offset += length;
            break;
        }
        case T_STRUCT: {
            const Struct* dstruct = dtype->as_struct();
            size_t num_fields = dstruct->get_num_fields();
            for(unsigned int i = 0; i < num_fields; ++i) {
                skip_dtype(dstruct->get_field(i)->get_type());
            }
            break;
        }
        case T_METHOD: {
            const Method* dmethod = dtype->as_method();
            size_t num_params = dmethod->get_num_parameters();
            for(unsigned int i = 0; i < num_params; ++i) {
                skip_dtype(dmethod->get_parameter(i)->get_type());
            }
            break;
        }
        default: {
            // This case should be impossible, but a default is required by compilers
            break;
        }
        }
    }

    // get_recipient_count returns the datagram's recipient count. Does not advance the offset.
    // Should be used when the current offset needs to be saved and/or if the next field in the
    // datagram is not the recipient_count. If stepping through a fresh datagram, use read_uint8.
    uint8_t get_recipient_count() const
    {
        if(m_dg->size() > 0) {
            return *(uint8_t*)(m_dg->get_data());
        } else {
            throw DatagramIteratorEOF("Cannot read header from empty datagram.");
        }
    }

    // get_sender returns the datagram's sender. Does not advance the offset.
    // Should be used when the current offset needs to be saved and/or if the next field in the
    //     datagram is not the sender. If stepping through a fresh datagram, use read_channel.
    channel_t get_sender()
    {
        dgsize_t offset = m_offset; // save offset

        m_offset = 1 + get_recipient_count() * sizeof(channel_t); // seek sender
        channel_t sender = read_channel(); // read sender

        m_offset = offset; // restore offset
        return sender;
    }

    // get_msg_type returns the datagram's message type. Does not advance the offset.
    // Should be used when the current offset needs to be saved and/or if the next field in the
    //     datagram is not the msg_type. If stepping through a fresh datagram, use read_uint16.
    uint16_t get_msg_type()
    {
        dgsize_t offset = m_offset; // save offset

        m_offset = 1 + get_recipient_count() * sizeof(channel_t) + sizeof(channel_t); // seek message type
        uint16_t msg_type = read_uint16(); // read message type

        m_offset = offset; // restore offset
        return msg_type;
    }

    // tell returns the current message offset in std::vector<uint8_t>
    dgsize_t tell() const
    {
        return m_offset;
    }

    // get_remaining returns the number of unread bytes left
    dgsize_t get_remaining() const
    {
        return m_dg->size() - m_offset;
    }

    // seek sets the current message offset in std::vector<uint8_t>
    void seek(dgsize_t to)
    {
        m_offset = to;
    }

    // seek_payload seeks to immediately after the list of receivers
    //     (typically, [channel_t sender, uint16_t msgtype, ...])
    void seek_payload()
    {
        m_offset = 0; // Seek to start
        m_offset = 1 + get_recipient_count() * sizeof(channel_t);
    }

    // skip increments the current message offset by a length.
    //     Throws DatagramIteratorEOF if it skips past the end of the datagram.
    void skip(dgsize_t length)
    {
        check_read_length(length);
        m_offset += length;
    }
};
