#pragma once
#include "Datagram.h"
#include "dcparser/dcClass.h"
#include <exception>
#include <stdexcept>
#include <sstream>

#ifdef _DEBUG
#include <fstream>
#endif

class DatagramIterator
{
private:
	const Datagram &m_dg;
	unsigned int m_offset;

	void check_read_length(unsigned int length)
	{
		if(m_offset+length > m_dg.size())
		{
			std::stringstream error;
			error << "dgi tried to read past dg end, offset+length(" << m_offset+length << ") buf_size(" << m_dg.size() << ")" << std::endl;
			#ifdef _DEBUG
				std::fstream test("test", std::ios_base::out | std::ios_base::binary);
				test.write(m_dg.get_data(), m_dg.get_buf_end());
				test.close();
			#endif
			throw std::runtime_error(error.str());
		};
	}
public:
	DatagramIterator(const Datagram &dg, unsigned int offset = 0) : m_dg(dg), m_offset(offset)
	{
		check_read_length(0); //shortcuts, yay
	}

	unsigned char read_uint8()
	{
		check_read_length(1);
		unsigned char r = *(unsigned char*)(m_dg.get_data()+m_offset);
		m_offset += 1;
		return r;
	}

	unsigned short read_uint16()
	{
		check_read_length(2);
		unsigned short r = *(unsigned short*)(m_dg.get_data()+m_offset);
		m_offset += 2;
		return r;
	}

	unsigned int read_uint32()
	{
		check_read_length(4);
		unsigned int r = *(unsigned int*)(m_dg.get_data()+m_offset);
		m_offset += 4;
		return r;
	}

	unsigned long long read_uint64()
	{
		check_read_length(8);
		unsigned long long r = *(unsigned long long*)(m_dg.get_data()+m_offset);
		m_offset += 8;
		return r;
	}

	// read_string reads a string from the datagram in format [len][<string-bytes>].
	// OTP messages are prefixed with a length, so this can be used to read the entire
	//     datagram, primarily useful to archive a datagram for later processing.
	std::string read_string()
	{
		unsigned int length = read_uint16();
		check_read_length(length);
		std::string str((char*)(m_dg.get_data()+m_offset), length);
		m_offset += length;
		return str;
	}

	bytes read_data(unsigned int length)
	{
		check_read_length(length);
		std::vector<unsigned char> data(m_dg.get_data()+m_offset, m_dg.get_data()+m_offset+length);
		m_offset += length;
		return data;
	}

	bytes read_remainder()
	{
		return read_data(m_dg.size() - m_offset);
	}

	void unpack_field(DCPackerInterface *field, bytes &buffer)
	{
		// If field is a fixed-sized type like uint, int, float, etc
		if(field->has_fixed_byte_size())
		{
			bytes data = read_data(field->get_fixed_byte_size());
			buffer.insert(buffer.end(), data.begin(), data.end());
			return;
		}

		// If field is a variable-sized type like string, blob, etc type with a "length" prefix
		unsigned int length = field->get_num_length_bytes();
		if(length > 0)
		{
			// Read length of field data
			switch(length)
			{
			case 2:
			{
				unsigned short l = read_uint16();
				buffer.insert(buffer.end(), (unsigned char*)&l, (unsigned char*)&l + 2);
				length = l;
			}
			break;
			case 4:
			{
				unsigned int l = read_uint32();
				buffer.insert(buffer.end(), (unsigned char*)&l, (unsigned char*)&l + 4);
				length = l;
			}
			break;
			}

			// Read field data into buffer
			bytes data = read_data(length);
			buffer.insert(buffer.end(), data.begin(), data.end());
			return;
		}

		// If field is non-atomic, process each nested field
		unsigned int num_nested = field->get_num_nested_fields();
		for(unsigned int i = 0; i < num_nested; ++i)
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
		unsigned int length = field->get_num_length_bytes();
		if(field->get_num_length_bytes() > 0)
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
		unsigned int num_nested = field->get_num_nested_fields();
		for(unsigned int i = 0; i < num_nested; ++i)
		{
			skip_field(field->get_nested_field(i));
		}
	}

	// tell returns the current message offset in bytes
	unsigned int tell()
	{
		return m_offset;
	}

	// seek sets the current message offset in bytes
	void seek(unsigned int to)
	{
		m_offset = to;
	}
};
