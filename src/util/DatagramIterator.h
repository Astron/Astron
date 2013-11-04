#pragma once
#include "Datagram.h"
#include "dcparser/dcClass.h"
#include <exception>
#include <stdexcept>
#include <sstream>

#ifdef _DEBUG
#include <fstream>
#endif

class DatagramIteratorEOF : public std::runtime_error
{
	public:
		DatagramIteratorEOF(const string &what) : std::runtime_error(what) { }
};

class DatagramIterator
{
	private:
		const Datagram &m_dg;
		dgsize_t m_offset;

		void check_read_length(dgsize_t length)
		{
			if(m_offset + length > m_dg.size())
			{
				std::stringstream error;
				error << "dgi tried to read past dg end, offset+length(" << m_offset + length << ")"
				      << " buf_size(" << m_dg.size() << ")" << std::endl;
				throw DatagramIteratorEOF(error.str());
			};
		}
	public:
		DatagramIterator(const Datagram &dg, dgsize_t offset = 0) : m_dg(dg), m_offset(offset)
		{
			check_read_length(0); //shortcuts, yay
		}

		bool read_bool()
		{
			uint8_t val = read_uint8();
			return val != false; // returns either 1 or 0
		}

		uint8_t read_uint8()
		{
			check_read_length(1);
			uint8_t r = *(uint8_t*)(m_dg.get_data() + m_offset);
			m_offset += 1;
			return r;
		}

		uint16_t read_uint16()
		{
			check_read_length(2);
			uint16_t r = *(uint16_t*)(m_dg.get_data() + m_offset);
			m_offset += 2;
			return r;
		}

		uint32_t read_uint32()
		{
			check_read_length(4);
			uint32_t r = *(uint32_t*)(m_dg.get_data() + m_offset);
			m_offset += 4;
			return r;
		}

		uint64_t read_uint64()
		{
			check_read_length(8);
			uint64_t r = *(uint64_t*)(m_dg.get_data() + m_offset);
			m_offset += 8;
			return r;
		}

		channel_t read_channel()
		{
			check_read_length(CHANNEL_SIZE_BYTES);
			channel_t r = *(channel_t*)(m_dg.get_data() + m_offset);
			m_offset += CHANNEL_SIZE_BYTES;
			return r;
		}

		doid_t read_doid()
		{
			check_read_length(DOID_SIZE_BYTES);
			doid_t r = *(doid_t*)(m_dg.get_data() + m_offset);
			m_offset += DOID_SIZE_BYTES;
			return r;
		}

		zone_t read_zone()
		{
			check_read_length(ZONE_SIZE_BYTES);
			zone_t r = *(zone_t*)(m_dg.get_data() + m_offset);
			m_offset += ZONE_SIZE_BYTES;
			return r;
		}

		// read_string reads a string from the datagram in the format
		//     {uint16 length; char[length] characters}.
		std::string read_string()
		{
			uint16_t length = read_uint16();
			check_read_length(length);
			std::string str((char*)(m_dg.get_data() + m_offset), length);
			m_offset += length;
			return str;
		}

		// read_blob reads a blob from the datagram in the format
		//     {uint16 length; uint8[length] binary}.
		std::vector<uint8_t> read_blob()
		{
			uint16_t length = read_uint16();
			return read_data(length);
		}

		// read_datagram reads a nested datagram from the datagram in the format
		//     {dgsize_t length; uint8[length] binary}.
		// This is also useful to read the whole datagram, because the beginning of the datagram
		// looks exactly the same as a nested datagram.
		std::vector<uint8_t> read_datagram()
		{
			check_read_length(DGSIZE_SIZE_BYTES);
			dgsize_t length = *(dgsize_t*)(m_dg.get_data() + m_offset);
			m_offset += DGSIZE_SIZE_BYTES;

			return read_data(length);
		}

		std::vector<uint8_t> read_data(dgsize_t length)
		{
			check_read_length(length);
			std::vector<uint8_t> data(m_dg.get_data() + m_offset, m_dg.get_data() + m_offset + length);
			m_offset += length;
			return data;
		}

		std::vector<uint8_t> read_remainder()
		{
			return read_data(m_dg.size() - m_offset);
		}

		void unpack_field(DCPackerInterface *field, std::vector<uint8_t> &buffer)
		{
			// If field is a fixed-sized type like uint, int, float, etc
			if(field->has_fixed_byte_size())
			{
				std::vector<uint8_t> data = read_data(field->get_fixed_byte_size());
				buffer.insert(buffer.end(), data.begin(), data.end());
				return;
			}

			// If field is a variable-sized type like string, blob, etc type with a "length" prefix
			size_t length = field->get_num_length_bytes();
			if(length > 0)
			{
				// Read length of field data
				switch(length)
				{
					case 2:
					{
						uint16_t l = read_uint16();
						buffer.insert(buffer.end(), (uint8_t*)&l, (uint8_t*)&l + 2);
						length = l;
					}
					break;
					case 4:
					{
						uint32_t l = read_uint32();
						buffer.insert(buffer.end(), (uint8_t*)&l, (uint8_t*)&l + 4);
						length = l;
					}
					break;
				}

				// Read field data into buffer
				std::vector<uint8_t> data = read_data(length);
				buffer.insert(buffer.end(), data.begin(), data.end());
				return;
			}

			// If field is non-atomic, process each nested field
			int num_nested = field->get_num_nested_fields();
			for(int i = 0; i < num_nested; ++i)
			{
				unpack_field(field->get_nested_field(i), buffer);
			}
		}

		void skip_field(DCPackerInterface *field)
		{
			// Skip over fields with fixed byte size
			if(field->has_fixed_byte_size())
			{
				check_read_length(field->get_fixed_byte_size());
				m_offset += field->get_fixed_byte_size();
				return;
			}

			// Skip over fields with variable byte size
			size_t length = field->get_num_length_bytes();
			if(length > 0)
			{
				// Get length of data
				switch(length)
				{
					case 2:
					{
						length = read_uint16();
					}
					break;
					case 4:
					{
						length = read_uint32();
					}
					break;
				}

				// Skip over data
				check_read_length(length);
				m_offset += length;
				return;
			}
			// If field is non-atomic, process each nested field
			int num_nested = field->get_num_nested_fields();
			for(int i = 0; i < num_nested; ++i)
			{
				skip_field(field->get_nested_field(i));
			}
		}

		// get_msg_type returns the datagram's recipient count. Does not advance the offset.
		// Should be used when the current offset needs to be saved and/or if the next field in the
		//     datagram is not the recipient_count. If stepping through a fresh datagram, use read_uint8().
		uint8_t get_recipient_count()
		{
			if(m_dg.size() > 0)
			{
				return *(uint8_t*)(m_dg.get_data());
			}
			else
			{
				throw DatagramIteratorEOF("Cannot read header from empty datagram.");
			}
		}

		// get_sender returns the datagram's sender. Does not advance the offset.
		// Should be used when the current offset needs to be saved and/or if the next field in the
		//     datagram is not the sender. If stepping through a fresh datagram, use read_uint64().
		channel_t get_sender()
		{
			uint16_t offset = m_offset; // save offset

			m_offset = 1 + get_recipient_count() * CHANNEL_SIZE_BYTES; // seek sender
			channel_t sender = read_channel(); // read sender

			m_offset = offset; // restore offset
			return sender;
		}

		// get_msg_type returns the datagram's message type. Does not advance the offset.
		// Should be used when the current offset needs to be saved and/or if the next field in the
		//     datagram is not the msg_type. If stepping through a fresh datagram, use read_uint16().
		uint16_t get_msg_type()
		{
			uint16_t offset = m_offset; // save offset

			m_offset = 9 + get_recipient_count() * CHANNEL_SIZE_BYTES; // seek message type
			uint16_t msg_type = read_uint16(); // read message type

			m_offset = offset; // restore offset
			return msg_type;
		}

		// tell returns the current message offset in std::vector<uint8_t>
		uint16_t tell()
		{
			return m_offset;
		}

		// get_remaining returns the number of unread bytes left
		uint16_t get_remaining() const
		{
			return m_dg.size() - m_offset;
		}

		// seek sets the current message offset in std::vector<uint8_t>
		void seek(dgsize_t to)
		{
			m_offset = to;
		}

		// seek_payload seeks to immediately after the list of receivers
		void seek_payload()
		{
			m_offset = 0; // Seek to start
			m_offset = 1 + get_recipient_count() * CHANNEL_SIZE_BYTES;
		}

		// skip increments the current message offset by a length.
		//     Throws DatagramIteratorEOF if it skips past the end of the datagram.
		void skip(dgsize_t length)
		{
			check_read_length(length);
			m_offset += length;
		}
};
