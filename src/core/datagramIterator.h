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
				throw std::runtime_error("dgi initialized with offset past dg end");
			};
		}
	public:
		DatagramIterator(Datagram *dg, unsigned int offset = 0) : m_dg(dg), p(offset)
		{
			check_read_length(0); //shortcuts, yay
		}
};