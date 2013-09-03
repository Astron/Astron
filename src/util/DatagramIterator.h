#pragma once
#include "Datagram.h"
#include <exception>
#include <stdexcept>
#include <sstream>
#ifdef _DEBUG
#include <fstream>
#endif

class DatagramIterator
{
	private:
		Datagram *m_dg;
		unsigned int p;

		void check_read_length(unsigned int l)
		{
			if(p+l > m_dg->get_buf_end())
			{
				std::stringstream error;
				error << "dgi tried to read past dg end, p+l(" << p+l << ") buf_end(" << m_dg->get_buf_end() << ")" << std::endl;
				#ifdef _DEBUG
					std::fstream test("test", std::ios_base::out | std::ios_base::binary);
					test.write(m_dg->get_data(), m_dg->get_buf_end());
					test.close();
				#endif
				throw std::runtime_error(error.str());
			};
		}
	public:
		DatagramIterator(Datagram *dg, unsigned int offset = 0) : m_dg(dg), p(offset)
		{
			check_read_length(0); //shortcuts, yay
		}

		unsigned char read_uint8()
		{
			check_read_length(1);
			unsigned char r = *(unsigned char*)(m_dg->get_data()+p);
			p += 1;
			return r;
		}

		unsigned short read_uint16()
		{
			check_read_length(2);
			unsigned short r = *(unsigned short*)(m_dg->get_data()+p);
			p += 2;
			return r;
		}

		unsigned long long read_uint64()
		{
			check_read_length(8);
			unsigned long long r = *(unsigned long long*)(m_dg->get_data()+p);
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
			std::string r(m_dg->get_data()+p, length);
			p += length;
			return r;
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
