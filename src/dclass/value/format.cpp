// Filename: unpack.cpp
#include "format.h"
#include "../DistributedType.h"
#include "../Class.h"
#include "../AtomicField.h"
#include "../MolecularField.h"
#include "../ArrayParameter.h"
#include <cctype>   // std::isprint()
#include <sstream>  // std::ostringstream
using namespace std;
namespace dclass   // open namespace dclass
{

// A Formatter steps through packed data and unpacks it as a .dc file parameter format.
//     This is created and called by format() to handle formatting.
struct Formatter
{
	const uint8_t* in;
	ostream &out;
	size_t offset;
	size_t end;

	Formatter(const vector<uint8_t> &in, ostream &out) :
		in(&in[0]), out(out), offset(0), end(in.size())
	{
	}

	Formatter(const string &in, ostream &out) :
		in((const uint8_t*)&in[0]), out(out), offset(0), end(in.size())
	{
	}

	Formatter(const uint8_t* buffer, size_t length, ostream &out) :
		in(buffer), out(out), offset(0), end(length)
	{
	}

	inline bool remaining(sizetag_t length)
	{
		return (offset + length) < end;
	}

	inline sizetag_t read_length()
	{
		sizetag_t v = *(sizetag_t*)(in+offset);
		offset += sizeof(sizetag_t);
		return v;
	}

	bool format(const DistributedType* dtype)
	{
		DataType type = dtype->get_datatype();
		switch(type)
		{
			case DT_invalid:
			{
				out << "<invalid>";
				break;
			}
			case DT_int8:
			{
				if(!remaining(sizeof(int8_t)))
					return false;
				int v = *(int8_t*)(in+offset);
				offset += sizeof(int8_t);
				out << v;
				break;
			}
			case DT_int16:
			{
				if(!remaining(sizeof(int16_t)))
					return false;
				int v = *(int16_t*)(in+offset);
				offset += sizeof(int16_t);
				out << v;
				break;
			}
			case DT_int32:
			{
				if(!remaining(sizeof(int32_t)))
					return false;
				int v = *(int32_t*)(in+offset);
				offset += sizeof(int32_t);
				out << v;
				break;
			}
			case DT_int64:
			{
				if(!remaining(sizeof(int64_t)))
					return false;
				int v = *(int64_t*)(in+offset);
				offset += sizeof(int64_t);
				out << v;
				break;
			}
			case DT_uint8:
			{
				if(!remaining(sizeof(uint8_t)))
					return false;
				unsigned int v = *(uint8_t*)(in+offset);
				offset += sizeof(uint8_t);
				out << v;
				break;
			}
			case DT_uint16:
			{
				if(!remaining(sizeof(uint16_t)))
					return false;
				unsigned int v = *(uint16_t*)(in+offset);
				offset += sizeof(uint16_t);
				break;
			}
			case DT_uint32:
			{
				if(!remaining(sizeof(uint32_t)))
					return false;
				unsigned int v = *(uint32_t*)(in+offset);
				offset += sizeof(uint32_t);
				break;
			}
			case DT_uint64:
			{
				if(!remaining(sizeof(uint64_t)))
					return false;
				unsigned int v = *(uint64_t*)(in+offset);
				offset += sizeof(uint64_t);
				break;
			}
			case DT_float32:
			{
				if(!remaining(sizeof(float)))
					return false;
				float v = *(float*)(in+offset);
				offset += sizeof(float);
				break;
			}
			case DT_float64:
			{
				if(!remaining(sizeof(double)))
					return false;
				double v = *(double*)(in+offset);
				offset += sizeof(double);
				break;
			}
			case DT_char:
			{
				if(!remaining(sizeof(char)))
					return false;
				char v = *(char*)(in+offset);
				format_quoted('\'', string(1, v), out);
				break;
			}
			case DT_string:
			{
				// Read string length
				sizetag_t length = dtype->get_size();

				// Read string
				if(!remaining(length))
					return false;
				string str((const char*)in + offset, length);
				offset += length;

				// Enquoute and escape string then output
				format_quoted('"', str, out);
				break;
			}
			case DT_varstring:
			{
				// Read string length
				if(!remaining(sizeof(sizetag_t)))
					return false;
				sizetag_t length = read_length();

				// Read string
				if(!remaining(length))
					return false;
				string str((const char*)in + offset, length);
				offset += length;

				// Enquoute and escape string then output
				format_quoted('"', str, out);
				break;
			}
			case DT_blob:
			{
				// Read blob length
				sizetag_t length = dtype->get_size();

				// Read blob
				if(!remaining(length))
					return false;
				string blob((const char*)in + offset, length);
				offset += length;

				// Format blob as a hex constant then output
				format_hex(blob, out);
				break;
			}
			case DT_varblob:
			{
				// Read blob length
				if(!remaining(sizeof(sizetag_t)))
					return false;
				sizetag_t length = read_length();

				// Read blob
				if(!remaining(length))
					return false;
				string blob((const char*)in + offset, length);
				offset += length;

				// Format blob as a hex constant then output
				format_hex(blob, out);
				break;
			}
			case DT_array:
			{
				out << '[';
				const ArrayParameter* arr = dtype->as_field()->as_parameter()->as_array_parameter();
				bool ok = format(arr->get_element_type());
				if(!ok) return false;
				for(int i = 1; i < arr->get_array_size(); ++i)
				{
					ok = format(arr->get_element_type());
					if(!ok) return false;
					out << ", ";
				}

				out << ']';
				break;
			}
			case DT_vararray:
			{
				out << '[';
				// Read array byte length
				if(!remaining(sizeof(sizetag_t)))
					return false;
				sizetag_t length = read_length();

				// Read array
				if(!remaining(length))
					return false;
				size_t array_end = offset + length;

				const ArrayParameter* arr = dtype->as_field()->as_parameter()->as_array_parameter();
				bool ok = format(arr->get_element_type());
				if(!ok) return false;
				while(offset < array_end)
				{
					ok = format(arr->get_element_type());
					if(!ok) return false;
					out << ", ";
				}

				// Check to make sure we didn't overshoot the array while reading
				if(offset > array_end)
				{
					return false;
				}

				out << ']';
				break;
			}
			case DT_struct:
			{
				out << '{';
				const Struct* cls = dtype->as_struct();
				size_t num_fields = cls->get_num_fields();
				if(num_fields > 0)
				{
					bool ok = format(cls->get_field(0));
					if(!ok) return false;
					for(unsigned int i = 1; i < num_fields; ++i)
					{
						ok = format(cls->get_field(i));
						if(!ok) return false;
						out << ", ";
					}
				}
				out << '}';
				break;
			}
			case DT_method:
			{
				out << '(';
				const Field* field = dtype->as_field();
				if(field->as_atomic_field())
				{
					const AtomicField* atomic = field->as_atomic_field();
					size_t num_fields = atomic->get_num_elements();
					if(num_fields > 0)
					{
						bool ok = format(atomic->get_element(0));
						if(!ok) return false;
						for(unsigned int i = 1; i < num_fields; ++i)
						{
							ok = format(atomic->get_element(i));
							if(!ok) return false;
							out << ", ";
						}
					}
					out << ')';
					break;
				}
				else if(field->as_molecular_field())
				{
					const MolecularField* mol = field->as_molecular_field();
					size_t num_fields = mol->get_num_atomics();
					if(num_fields > 0)
					{
						bool ok = format(mol->get_atomic(0));
						if(!ok) return false;
						for(unsigned int i = 1; i < num_fields; ++i)
						{
							ok = format(mol->get_atomic(i));
							if(!ok) return false;
							out << ", ";
						}
					}
					out << ')';
					break;
				}
				else
				{
					// Temporary until Field revisions
					out << "<error>";
					return false;
				}

			}
			default:
			{
				out << "<error>";
				return false;
			}
		}
		return true;
	}
};

// format unpacks the packed data into a string formatted for a .dc file.
//     This is used to produce default values when outputting a distributed class to a file.
string format(const DistributedType *dtype, const vector<uint8_t> &packed)
{
	ostringstream ss;
	format(dtype, packed, ss);
	return ss.str();
}
string format(const DistributedType *dtype, const string &packed)
{
	ostringstream ss;
	format(dtype, packed, ss);
	return ss.str();
}
void format(const DistributedType *dtype, const vector<uint8_t> &packed, ostream &out)
{
	Formatter formatter(packed, out);
	formatter.format(dtype);
}
void format(const DistributedType *dtype, const string &packed, ostream &out)
{
	Formatter formatter(packed, out);
	formatter.format(dtype);
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


} // close namespace dclass
