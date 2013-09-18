#pragma once
#include <set>
#include <string>
#include <vector>
#include <string.h> // memcpy
#include "core/messages.h"

typedef std::vector<unsigned char> bytes;

class Datagram
{
private:
	unsigned char* buf;
	unsigned int buf_size;
	unsigned int buf_end;

	void check_add_length(unsigned int len)
	{
		if(buf_end+len > buf_size)
		{
			unsigned char *tmp_buf = new unsigned char[buf_size+len+64];
			memcpy(tmp_buf, buf, buf_size);
			delete [] buf;
			buf = tmp_buf;
		}
	}
public:
	Datagram() : buf(new unsigned char[64]), buf_size(64), buf_end(0)
	{
	}

	Datagram(const std::string &data) : buf(new unsigned char[data.length()]), buf_size(data.length()), buf_end(data.length())
	{
		memcpy(buf, data.c_str(), data.length());
	}

	Datagram(const char *data, size_t length) : buf(new unsigned char[length]), buf_size(length), buf_end(length)
	{
		memcpy(buf, data, length);
	}

	Datagram(unsigned long long to_channel, unsigned long long from_channel, unsigned short message_type) : buf(new unsigned char[64]), buf_size(64), buf_end(0)
	{
		add_server_header(to_channel, from_channel, message_type);
	}

	Datagram(const std::set<unsigned long long> &to_channels, unsigned long long from_channel, unsigned short message_type) : buf(new unsigned char[64]), buf_size(64), buf_end(0)
	{
		add_server_header(to_channels, from_channel, message_type);
	}

	Datagram(unsigned short message_type) : buf(new unsigned char[64]), buf_size(64), buf_end(0)
	{
		add_control_header(message_type);
	}

	~Datagram()
	{
		delete [] buf;
	}

	void add_uint8(const unsigned char &v)
	{
		check_add_length(1);
		memcpy(buf+buf_end, &v, 1);
		buf_end += 1;
	}

	void add_uint16(const unsigned short &v)
	{
		check_add_length(2);
		memcpy(buf+buf_end, &v, 2);
		buf_end += 2;
	}

	void add_uint32(const unsigned int &v)
	{
		check_add_length(4);
		memcpy(buf+buf_end, &v, 4);
		buf_end += 4;
	}

	void add_uint64(const unsigned long long &v)
	{
		check_add_length(8);
		memcpy(buf+buf_end, &v, 8);
		buf_end += 8;
	}

	void add_data(const bytes &data)
	{
		check_add_length(data.size());
		memcpy(buf+buf_end, &data[0], data.size());
		buf_end += data.size();
	}

	void add_datagram(const Datagram &dg)
	{
		check_add_length(dg.buf_end);
		memcpy(buf+buf_end, dg.buf, dg.buf_end);
		buf_end += dg.buf_end;
	}

	void add_string(const std::string &str)
	{
		add_uint16(str.length());
		check_add_length(str.length());
		memcpy(buf+buf_end, str.c_str(), str.length());
		buf_end += str.length();
	}

	void add_server_header(channel_t to, channel_t from, unsigned short message_type)
	{
		add_uint8(1);
		add_uint64(to);
		add_uint64(from);
		add_uint16(message_type);
	}

	void add_server_header(const std::set<channel_t> &to, channel_t from, unsigned short message_type)
	{
		add_uint8(to.size());
		for(auto it = to.begin(); it != to.end(); ++it)
			add_uint64(*it);
		add_uint64(from);
		add_uint16(message_type);
	}

	void add_control_header(unsigned short message_type)
	{
		add_uint8(1);
		add_uint64(CONTROL_MESSAGE);
		add_uint16(message_type);
	}

	unsigned int size() const
	{
		return buf_end;
	}

	unsigned char* get_data() const
	{
		return buf;
	}
};
