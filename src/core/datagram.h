#pragma once

class Datagram
{
	private:
		char* buf;
		unsigned int p;
		unsigned int buf_size;
	public:
		Datagram() : buf(new char[64]), p(0), buf_size(64)
		{
		}
};