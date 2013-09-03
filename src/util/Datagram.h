#pragma once
#include <string>
#include <string.h>

class Datagram
{
	private:
		char* buf;
		unsigned int p;
		unsigned int buf_size;
		unsigned int buf_end;

		void check_add_length(unsigned int len)
		{
			if(p+len > buf_size)
			{
				char *tmp_buf = new char[buf_size+len+64];
				memcpy(tmp_buf, buf, buf_size);
				delete [] buf;
			}
		}
	public:
		Datagram() : buf(new char[64]), p(0), buf_size(64), buf_end(0)
		{
		}

		Datagram(const std::string &data) : buf(new char[data.length()]), p(0), buf_size(data.length()), buf_end(data.length())
		{
			memcpy(buf, data.c_str(), data.length());
		}

		void add_uint8(const unsigned char &v)
		{
			check_add_length(1);
			memcpy(buf+p, &v, 1);
			p += 1;
			buf_end += 1;
		}

		void add_uint16(const unsigned short &v)
		{
			check_add_length(2);
			memcpy(buf+p, &v, 2);
			p += 2;
			buf_end += 2;
		}

		void add_uint32(const unsigned int &v)
		{
			check_add_length(4);
			memcpy(buf+p, &v, 4);
			p += 4;
			buf_end += 4;
		}

		void add_uint64(const unsigned long long &v)
		{
			check_add_length(8);
			memcpy(buf+p, &v, 8);
			p += 8;
			buf_end += 8;
		}

		void add_data(const std::string &data)
		{
			check_add_length(data.length());
			memcpy(buf+p, data.c_str(), data.length());
			p += data.length();
			buf_end += data.length();
		}

		void add_string(const std::string &str)
		{
			add_uint16(str.length());
			add_data(str);
		}

		unsigned int get_buf_end()
		{
			return buf_end;
		}

		const char* get_data()
		{
			return buf;
		}
};