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
		uint16_t m_offset;

		void check_read_length(uint16_t length)
		{
			if(m_offset + length > m_dg.size())
			{
				std::stringstream error;
				error << "dgi tried to read past dg end, offset+length(" << m_offset + length << ")"
				      << " buf_size(" << m_dg.size() << ")" << std::endl;
#ifdef _DEBUG
				std::fstream test("test", std::ios_base::out | std::ios_base::binary);
				test.write(m_dg.get_data(), m_dg.get_buf_end());
				test.close();
#endif
				throw DatagramIteratorEOF(error.str());
			};
		}
	public:
		DatagramIterator(const Datagram &dg, uint16_t offset = 0) : m_dg(dg), m_offset(offset)
		{
			check_read_length(0); //shortcuts, yay
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

		// read_string reads a string from the datagram in format [len][<string-std::vector<uint8_t>>].
		// OTP messages are prefixed with a length, so this can be used to read the entire
		//     datagram, primarily useful to archive a datagram for later processing.
		std::string read_string()
		{
			uint32_t length = read_uint16();
			check_read_length(length);
			std::string str((char*)(m_dg.get_data() + m_offset), length);
			m_offset += length;
			return str;
		}

		std::vector<uint8_t> read_data(uint32_t length)
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

		// tell returns the current message offset in std::vector<uint8_t>
		uint16_t tell()
		{
			return m_offset;
		}

		// seek sets the current message offset in std::vector<uint8_t>
		void seek(uint16_t to)
		{
			m_offset = to;
		}

		// seek_payload seeks to immediately after the list of receivers
		void seek_payload()
		{
			m_offset = 0;
			uint8_t recv_count = read_uint8();
			m_offset = 1 + recv_count * 8;
		}
};
