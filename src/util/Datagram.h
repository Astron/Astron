#pragma once
#include <string>
#include <set>
#include <string.h>

#define CONTROL_MESSAGE 4001

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

		Datagram(unsigned long long to_channel, unsigned long long from_channel, unsigned short message_type) : buf(new char[64]), p(0), buf_size(64), buf_end(0)
		{
			add_server_header(to_channel, from_channel, message_type);
		}

		Datagram(const std::set<unsigned long long> &to_channels, unsigned long long from_channel, unsigned short message_type) : buf(new char[64]), p(0), buf_size(64), buf_end(0)
		{
			add_server_header(to_channels, from_channel, message_type);
		}

		Datagram(unsigned short message_type) : buf(new char[64]), p(0), buf_size(64), buf_end(0)
		{
			add_control_header(message_type);
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

		void add_server_header(unsigned long long to_channel, unsigned long long from_channel, unsigned short message_type)
		{
			add_uint8(1);
			add_uint64(to_channel);
			add_uint64(from_channel);
			add_uint16(message_type);
		}

		void add_server_header(const std::set<unsigned long long> &to_channels, unsigned long long from_channel, unsigned short message_type)
		{
			add_uint8(to_channels.size());
			for(auto it = to_channels.begin(); it != to_channels.end(); ++it)
				add_uint64(*it);
			add_uint64(from_channel);
			add_uint16(message_type);
		}

		void add_control_header(unsigned short message_type)
		{
			add_uint8(1);
			add_uint64(CONTROL_MESSAGE);
			add_uint16(message_type);
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
