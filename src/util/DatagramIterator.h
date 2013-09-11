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
	Datagram &m_dg;
	unsigned int p;

	void check_read_length(unsigned int l)
	{
		if(p+l > m_dg.get_buf_end())
		{
			std::stringstream error;
			error << "dgi tried to read past dg end, p+l(" << p+l << ") buf_end(" << m_dg.get_buf_end() << ")" << std::endl;
			#ifdef _DEBUG
				std::fstream test("test", std::ios_base::out | std::ios_base::binary);
				test.write(m_dg.get_data(), m_dg.get_buf_end());
				test.close();
			#endif
			throw std::runtime_error(error.str());
		};
	}
public:
	DatagramIterator(Datagram &dg, unsigned int offset = 0) : m_dg(dg), p(offset)
	{
		check_read_length(0); //shortcuts, yay
	}

	unsigned char read_uint8()
	{
		check_read_length(1);
		unsigned char r = *(unsigned char*)(m_dg.get_data()+p);
		p += 1;
		return r;
	}

	unsigned short read_uint16()
	{
		check_read_length(2);
		unsigned short r = *(unsigned short*)(m_dg.get_data()+p);
		p += 2;
		return r;
	}

	unsigned int read_uint32()
	{
		check_read_length(4);
		unsigned int r = *(unsigned int*)(m_dg.get_data()+p);
		p += 4;
		return r;
	}

	unsigned long long read_uint64()
	{
		check_read_length(8);
		unsigned long long r = *(unsigned long long*)(m_dg.get_data()+p);
		p += 8;
		return r;
	}

	// read_string reads a string from the datagram in format [len][<string-bytes>].
	// OTP messages are prefixed with a length, so this can be used to read the entire
	//     datagram, primarily useful to archive a datagram for later processing.
	std::string read_string()
	{
		unsigned int length = read_uint16();
		check_read_length(length);
		std::string r(m_dg.get_data()+p, length);
		p += length;
		return r;
	}

	std::string read_data(unsigned int length)
	{
		check_read_length(length);
		std::string r(m_dg.get_data()+p, length);
		p += length;
		return r;
	}

	std::string read_remainder()
	{
		return read_data(m_dg.get_buf_end() - p);
	}

	void unpack_field(DCPackerInterface *field, std::string &str)
	{
		if(field->has_fixed_byte_size())
		{
			str += read_data(field->get_fixed_byte_size());
		}
		else if(field->get_num_length_bytes() > 0)
		{
			unsigned int length = field->get_num_length_bytes();
			switch(length)
			{
			case 2:
			{
				unsigned short l = read_uint16();
				str += std::string((char*)&l, 2);
				length = l;
			}
			break;
			case 4:
			{
				unsigned int l = read_uint32();
				str += std::string((char*)&l, 4);
				length = l;
			}
			break;
			break;
			}
			str += read_data(length);
		}
		else
		{
			unsigned int nNested = field->get_num_nested_fields();
			for(unsigned int i = 0; i < nNested; ++i)
			{
				unpack_field(field->get_nested_field(i), str);
			}
		}
	}

	// tell returns the current message offset in bytes
	unsigned int tell()
	{
		return p;
	}

	// seek sets the current message offset in bytes
	void seek(unsigned int to)
	{
		p = to;
	}
};
