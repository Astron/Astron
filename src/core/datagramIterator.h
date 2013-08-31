#pragma once
#include "datagram.h"
#include <exception>

class DatagramIterator
{
	private:
		Datagram *m_dg;
		unsigned int p;

		void check_read_length(unsigned int l)
		{
			if(p+l > m_dg->get_buf_end())
			{
				throw std::runtime_error("dgi tried to read past dg end");
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
		}

		unsigned short read_uint16()
		{
			check_read_length(2);
			unsigned short r = *(unsigned short*)(m_dg->get_data()+p);
			p += 2;
		}

		std::string read_string()
		{
			unsigned int length = read_uint16();
			check_read_length(length);
			std::string r(m_dg->get_data()+p, length);
			p += length;
			return r;
		}
};