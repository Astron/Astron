#pragma once
#include "datagram.h"
#include <exception>

class DatagramIterator
{
	private:
		Datagram *m_dg;
		unsigned int p;

		void CheckReadLength(unsigned int l)
		{
			if(p+l > m_dg->GetBufEnd())
			{
				throw std::runtime_error("dgi initialized with offset past dg end");
			};
		}
	public:
		DatagramIterator(Datagram *dg, unsigned int offset = 0) : m_dg(dg), p(offset)
		{
			CheckReadLength(0); //shortcuts, yay
		}
};