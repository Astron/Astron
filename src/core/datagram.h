#pragma once
#include <string>

class Datagram
{
	private:
		char* buf;
		unsigned int p;
		unsigned int buf_size;

		void CheckAddLength(unsigned int len)
		{
			if(p+len > buf_size)
			{
				char *tmp_buf = new char[buf_size+len+64];
				memcpy(tmp_buf, buf, buf_size);
				delete [] buf;
			}
		}
	public:
		Datagram() : buf(new char[64]), p(0), buf_size(64)
		{
		}

		Datagram(const std::string &data) : buf(new char[data.length()]), p(0), buf_size(data.length())
		{
			memcpy(buf, data.c_str(), data.length());
		}

		void AddUint16(const unsigned short &v)
		{
			CheckAddLength(2);
			memcpy(buf+p, &v, 2);
			p += 2;
		}

		void AddData(const std::string &data)
		{
			CheckAddLength(data.length());
			memcpy(buf+p, data.c_str(), data.length());
			p += data.length();
		}

		void AddString(const std::string &str)
		{
			AddUint16(str.length());
			AddData(str);
		}
};