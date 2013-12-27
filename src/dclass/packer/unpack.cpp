// Filename: unpack.cpp

#include "unpack.h"
#include <cctype> // for isprint()
#include <sstream> // for ostringstream

using namespace std;
namespace dclass   // open namespace dclass
{


string format(const Element *element, const vector<uint8_t> &packed)
{
	ostringstream ss;
	format(element, packed, ss);
	return ss.str();
}

void format(const Element *element, const vector<uint8_t> &packed, ostream &out)
{
	Unpacker formatter(packed, out);
	formatter.format(element);
}

// format_hex outputs <str> to <out> as a hexidecimal constant enclosed in angle-brackets (<>).
void format_hex(const string &str, ostream &out)
{
	out << '<';
	for(auto it = str.begin(); it != str.end(); ++it)
	{
		char infer[10];
		sprintf(infer, "%02x", (unsigned char)(*it));
		out << infer;
	}
	out << '>';
}
string format_hex(const string &str)
{
	ostringstream ss;
	format_hex(str, ss);
	return ss.str();
}

// format_quoted outputs <str> to <out> quoted with the character <quote_mark>.
//     Any instances of backslash (\) or the quoute character in the string are escaped.
//     Non-printable characters are replaced with an escaped hexidecimal constant.
void format_quoted(char quote_mark, const string &str, ostream &out)
{
	out << quote_mark;
	for(auto it = str.begin(); it != str.end(); ++it)
	{
		char c = *it;
		if(c == quote_mark || c == '\\')
		{
			// escape the character
			out << '\\' << c;

		}
		else if(!isprint(c)) // character is not a printable ascii character
		{
			// print the character as an escaped hexidecimal character constant
			char infer[10];
			sprintf(infer, "%02x", (unsigned char)c);
			out << "\\x" << infer;
		}
		else
		{
			out << c;
		}
	}
	out << quote_mark;
}
string format_quoted(char quote_mark, const string &str)
{
	ostringstream ss;
	format_quoted(quote_mark, str, ss);
	return ss.str();
}


Unpacker::Unpacker(const vector<uint8_t> &in, ostream &out) : in(&in[0]), out(out), offset(0)
{
}

void Unpacker::format(const Element *element)
{
	DataType type = element->get_type();
	switch(type)
	{
		case DT_invalid:
			out << "<invalid>";
			break;
		case DT_char:
			format_quoted('\'', unpack_char(), out);
		case DT_string:
			format_quoted('"', unpack_string(element), out);
			break;
		case DT_varstring:
			format_quoted('"', unpack_varstring(), out);
			break;
		case DT_blob:
			format_hex(unpack_blob(element), out);
			break;
		case DT_varblob:
			format_hex(unpack_varblob(), out);
			break;
		case DT_array:
			out << '[';
			/*
			for element in array {
				format element
				print ", "
			}
			*/
			out << ']';
			break;
		case DT_struct:
			out << '{';
			/*
			for member of struct {
				format member
				print ", "
			}
			*/
			out << '}';
			break;
		case DT_method:
			out << '(';
			/*
			for argument of method {
				format argument
				print ", "
			}
			*/
			out << ')';
			break;
		default:
			bool is_number = format_as_number(element);
			if(!is_number)
			{
				out << "<error>";
				// error?
				// return false?
				// etc?
				break;
			}
	}
}

// TODO: doc
bool Unpacker::format_as_number(const Element* element)
{
	DataType type = element->get_type();
	switch(type)
	{
		case DT_float32:
			out << unpack_float32();
			break;
		case DT_float64:
			out << unpack_float64();
			break;
		case DT_int8:
			out << (int)unpack_int8();
			break;
		case DT_int16:
			out << unpack_int16();
			break;
		case DT_int32:
			out << unpack_int32();
			break;
		case DT_int64:
			out << unpack_int64();
			break;
		case DT_uint8:
			out << (unsigned int)unpack_uint8();
			break;
		case DT_uint16:
			out << unpack_uint16();
			break;
		case DT_uint32:
			out << unpack_uint32();
			break;
		case DT_uint64:
			out << unpack_uint64();
			break;
		default:
			return false;
	}

	return true;
}


uint8_t Unpacker::unpack_uint8()
{
	uint8_t v = *(uint8_t*)(in+offset);
	offset += sizeof(uint8_t);
	return v;
}
uint16_t Unpacker::unpack_uint16()
{
	uint16_t v = *(uint16_t*)(in+offset);
	offset += sizeof(uint16_t);
	return v;
}
uint32_t Unpacker::unpack_uint32()
{
	uint32_t v = *(uint32_t*)(in+offset);
	offset += sizeof(uint32_t);
	return v;
}
uint64_t Unpacker::unpack_uint64()
{
	uint64_t v = *(uint64_t*)(in+offset);
	offset += sizeof(uint64_t);
	return v;
}
int8_t Unpacker::unpack_int8()
{
	int8_t v = *(int8_t*)(in+offset);
	offset += sizeof(int8_t);
	return v;
}
int16_t Unpacker::unpack_int16()
{
	int16_t v = *(int16_t*)(in+offset);
	offset += sizeof(int16_t);
	return v;
}
int32_t Unpacker::unpack_int32()
{
	int32_t v = *(int32_t*)(in+offset);
	offset += sizeof(int32_t);
	return v;
}
int64_t Unpacker::unpack_int64()
{
	int64_t v = *(int64_t*)(in+offset);
	offset += sizeof(int64_t);
	return v;
}
float Unpacker::unpack_float32()
{
	float v = *(float*)(in+offset);
	offset += sizeof(float);
	return v;
}
double Unpacker::unpack_float64()
{
	double v = *(double*)(in+offset);
	offset += sizeof(double);
	return v;
}
length_tag_t Unpacker::unpack_length()
{
	length_tag_t v = *(length_tag_t*)(in+offset);
	offset += sizeof(length_tag_t);
}
string Unpacker::unpack_char()
{
	string v((const char *)in + offset, sizeof(char));
	offset += sizeof(char);
	return v;
}
string Unpacker::unpack_string(const Element* element)
{
	length_tag_t length = element->get_size();
	string v((const char *)in + offset, length);
	offset += length;
	return v;
}
string Unpacker::unpack_varstring()
{
	length_tag_t length = unpack_length();
	string v((const char *)in + offset, length);
	offset += length;
	return v;
}
string Unpacker::unpack_blob(const Element* element)
{
	length_tag_t length = element->get_size();
	string v((const char *)in + offset, length);
	offset += length;
	return v;
}
string Unpacker::unpack_varblob()
{
	length_tag_t length = unpack_length();
	string v((const char *)in + offset, length);
	offset += length;
	return v;
}


} // close namespace dclass
