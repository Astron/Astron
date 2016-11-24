// Filename: format.cpp
#include <cctype>  // std::isprint()
#include <sstream> // std::ostringstream
#include "dc/DistributedType.h"
#include "dc/ArrayType.h"
#include "dc/Struct.h"
#include "dc/Field.h"
#include "dc/Method.h"
#include "dc/Parameter.h"
#include "util/byteorder.h"

#if defined(_WIN32) && defined(_MSC_VER) && _MSC_VER <= 1800
#define snprintf sprintf_s
#endif


#include "format.h"
using namespace std;
namespace dclass   // open namespace dclass
{

// A Formatter steps through packed data and unpacks it as a .dc file parameter format.
//     This is created and called by format() to handle formatting.
struct Formatter {
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
        return (offset + length) <= end;
    }

    inline sizetag_t read_length()
    {
        sizetag_t v = swap_le(*(sizetag_t*)(in + offset));
        offset += sizeof(sizetag_t);
        return v;
    }

    bool format(const DistributedType* dtype)
    {
        Type type = dtype->get_type();
        switch(type) {
        case T_INVALID: {
            out << "<invalid>";
            break;
        }
        case T_INT8: {
            if(!remaining(sizeof(int8_t))) {
                return false;
            }
            int v = *(int8_t*)(in + offset);
            offset += sizeof(int8_t);
            out << v;
            break;
        }
        case T_INT16: {
            if(!remaining(sizeof(int16_t))) {
                return false;
            }
            int v = swap_le(*(int16_t*)(in + offset));
            offset += sizeof(int16_t);
            out << v;
            break;
        }
        case T_INT32: {
            if(!remaining(sizeof(int32_t))) {
                return false;
            }
            int v = swap_le(*(int32_t*)(in + offset));
            offset += sizeof(int32_t);
            out << v;
            break;
        }
        case T_INT64: {
            if(!remaining(sizeof(int64_t))) {
                return false;
            }
            int64_t v = swap_le(*(int64_t*)(in + offset));
            offset += sizeof(int64_t);
            out << v;
            break;
        }
        case T_UINT8: {
            if(!remaining(sizeof(uint8_t))) {
                return false;
            }
            unsigned int v = *(uint8_t*)(in + offset);
            offset += sizeof(uint8_t);
            out << v;
            break;
        }
        case T_UINT16: {
            if(!remaining(sizeof(uint16_t))) {
                return false;
            }
            unsigned int v = swap_le(*(uint16_t*)(in + offset));
            offset += sizeof(uint16_t);
            out << v;
            break;
        }
        case T_UINT32: {
            if(!remaining(sizeof(uint32_t))) {
                return false;
            }
            unsigned int v = swap_le(*(uint32_t*)(in + offset));
            offset += sizeof(uint32_t);
            out << v;
            break;
        }
        case T_UINT64: {
            if(!remaining(sizeof(uint64_t))) {
                return false;
            }
            uint64_t v = swap_le(*(uint64_t*)(in + offset));
            offset += sizeof(uint64_t);
            out << v;
            break;
        }
        case T_FLOAT32: {
            if(!remaining(sizeof(float))) {
                return false;
            }
            float v = (float)swap_le(*(float*)(in + offset));
            offset += sizeof(float);
            out << v;
            break;
        }
        case T_FLOAT64: {
            if(!remaining(sizeof(double))) {
                return false;
            }
            double v = (double)swap_le(*(double*)(in + offset));
            offset += sizeof(double);
            out << v;
            break;
        }
        case T_CHAR: {
            if(!remaining(sizeof(char))) {
                return false;
            }
            char v = *(char*)(in + offset);
            format_quoted('\'', string(1, v), out);
            offset += sizeof(char);
            break;
        }
        case T_STRING: {
            // Read string length
            sizetag_t length = dtype->get_size();


            // If we have a string alias format as a quoted string
            if(dtype->has_alias() && dtype->get_alias() == "string") {
                // Read string
                if(!remaining(length)) {
                    return false;
                }
                string str((const char*)in + offset, length);
                offset += length;

                // Enquoute and escape string then output
                format_quoted('"', str, out);
            } else {
                // Otherwise format as an array of char
                out << '[';
                const ArrayType* arr = dtype->as_array();
                bool ok = format(arr->get_element_type());
                if(!ok) {
                    return false;
                }
                for(unsigned int i = 1; i < arr->get_array_size(); ++i) {
                    out << ", ";
                    ok = format(arr->get_element_type());
                    if(!ok) {
                        return false;
                    }
                }

                out << ']';
            }
            break;
        }
        case T_VARSTRING: {

            // If we have a string alias format as a quoted string
            if(dtype->has_alias() && dtype->get_alias() == "string") {
                // Read string length
                if(!remaining(sizeof(sizetag_t))) {
                    return false;
                }
                sizetag_t length = read_length();

                // Read string
                if(!remaining(length)) {
                    return false;
                }
                string str((const char*)in + offset, length);
                offset += length;

                // Enquoute and escape string then output
                format_quoted('"', str, out);
            } else {
                // Otherwise format as an array of char
                out << '[';
                // Read array byte length
                if(!remaining(sizeof(sizetag_t))) {
                    out << ']';
                    return false;
                }
                sizetag_t length = read_length();

                if(length == 0) {
                    out << ']';
                    break;
                }

                // Read array
                if(!remaining(length)) {
                    out << ']';
                    return false;
                }
                size_t array_end = offset + length;

                const ArrayType* arr = dtype->as_array();
                bool ok = format(arr->get_element_type());
                if(!ok) {
                    out << ']';
                    return false;
                }

                while(offset < array_end) {
                    out << ", ";
                    ok = format(arr->get_element_type());
                    if(!ok) {
                        out << ']';
                        return false;
                    }
                }

                // Check to make sure we didn't overshoot the array while reading
                if(offset > array_end) {
                    out << ']';
                    return false;
                }

                out << ']';
            }

            break;
        }
        case T_BLOB: {
            // Read blob length
            sizetag_t length = dtype->get_size();

            // If we have a blob alias format as a hex constant
            if(dtype->has_alias() && dtype->get_alias() == "blob") {
                // Read blob
                if(!remaining(length)) {
                    return false;
                }
                string blob((const char*)in + offset, length);
                offset += length;

                // Format blob as a hex constant then output
                format_hex(blob, out);
            } else {
                // Otherwise format as an array of uint8
                out << '[';
                const ArrayType* arr = dtype->as_array();
                bool ok = format(arr->get_element_type());
                if(!ok) {
                    out << ']';
                    return false;
                }

                for(unsigned int i = 1; i < arr->get_array_size(); ++i) {
                    out << ", ";
                    ok = format(arr->get_element_type());
                    if(!ok) {
                        out << ']';
                        return false;
                    }
                }

                out << ']';
            }

            break;
        }
        case T_VARBLOB: {
            // If we have a blob alias format as a hex constant
            if(dtype->has_alias() && dtype->get_alias() == "blob") {
                // Read blob length
                if(!remaining(sizeof(sizetag_t))) {
                    return false;
                }
                sizetag_t length = read_length();


                // Read blob with length
                if(!remaining(length)) {
                    return false;
                }
                string blob((const char*)in + offset - 2, length + 2);
                offset += length;

                // Format blob and length as a hex constant then output
                format_hex(blob, out);
            } else {
                // Otherwise format as an array of uint8
                out << '[';
                // Read array byte length
                if(!remaining(sizeof(sizetag_t))) {
                    out << ']';
                    return false;
                }
                sizetag_t length = read_length();

                if(length == 0) {
                    out << ']';
                    break;
                }
                // Read array
                if(!remaining(length)) {
                    out << ']';
                    return false;
                }
                size_t array_end = offset + length;

                const ArrayType* arr = dtype->as_array();
                bool ok = format(arr->get_element_type());
                if(!ok) {
                    out << ']';
                    return false;
                }
                while(offset < array_end) {
                    out << ", ";
                    ok = format(arr->get_element_type());
                    if(!ok) {
                        out << ']';
                        return false;
                    }
                }

                // Check to make sure we didn't overshoot the array while reading
                if(offset > array_end) {
                    out << ']';
                    return false;
                }

                out << ']';
            }

            break;
        }
        case T_ARRAY: {
            out << '[';
            const ArrayType* arr = dtype->as_array();
            bool ok = format(arr->get_element_type());
            if(!ok) {
                out << ']';
                return false;
            }
            for(unsigned int i = 1; i < arr->get_array_size(); ++i) {
                out << ", ";
                ok = format(arr->get_element_type());
                if(!ok) {
                    out << ']';
                    return false;
                }
            }

            out << ']';
            break;
        }
        case T_VARARRAY: {
            out << '[';
            // Read array byte length
            if(!remaining(sizeof(sizetag_t))) {
                out << ']';
                return false;
            }
            sizetag_t length = read_length();

            if(length == 0) {
                out << ']';
                break;
            }

            // Read array
            if(!remaining(length)) {
                out << ']';
                return false;
            }
            size_t array_end = offset + length;

            const ArrayType* arr = dtype->as_array();
            bool ok = format(arr->get_element_type());
            if(!ok) {
                out << ']';
                return false;
            }
            while(offset < array_end) {
                out << ", ";
                ok = format(arr->get_element_type());
                if(!ok) {
                    out << ']';
                    return false;
                }
            }

            // Check to make sure we didn't overshoot the array while reading
            if(offset > array_end) {
                out << ']';
                return false;
            }

            out << ']';
            break;
        }
        case T_STRUCT: {
            out << '{';
            const Struct* strct = dtype->as_struct();
            size_t num_fields = strct->get_num_fields();
            if(num_fields > 0) {
                bool ok = format(strct->get_field(0)->get_type());
                if(!ok) {
                    out << '}';
                    return false;
                }
                for(unsigned int i = 1; i < num_fields; ++i) {
                    out << ", ";
                    ok = format(strct->get_field(i)->get_type());
                    if(!ok) {
                        out << '}';
                        return false;
                    }
                }
            }
            out << '}';
            break;
        }
        case T_METHOD: {
            out << '(';
            const Method* method = dtype->as_method();
            size_t num_params = method->get_num_parameters();
            if(num_params > 0) {
                bool ok = format(method->get_parameter(0)->get_type());
                if(!ok) {
                    out << ')';
                    return false;
                }
                for(unsigned int i = 1; i < num_params; ++i) {
                    out << ", ";
                    ok = format(method->get_parameter(i)->get_type());
                    if(!ok) {
                        out << ')';
                        return false;
                    }
                }
            }
            out << ')';
            break;
        }
        default: {
            out << "<error>";
            return false;
        }
        }
        return true;
    }
};

// format unpacks the packed data into a string formatted for a .dc file.
//     This is used to produce default values when outputting a distributed class to a file.
string format_value(const DistributedType *dtype, const vector<uint8_t> &packed)
{
    ostringstream ss;
    format_value(dtype, packed, ss);
    return ss.str();
}
string format_value(const DistributedType *dtype, const string &packed)
{
    ostringstream ss;
    format_value(dtype, packed, ss);
    return ss.str();
}
void format_value(const DistributedType *dtype, const vector<uint8_t> &packed, ostream &out)
{
    Formatter formatter(packed, out);
    formatter.format(dtype);
}
void format_value(const DistributedType *dtype, const string &packed, ostream &out)
{
    Formatter formatter(packed, out);
    formatter.format(dtype);
}

// format_hex outputs <str> to <out> as a hexidecimal constant enclosed in angle-brackets (<>).
void format_hex(const string &str, ostream &out)
{
    out << '<';
    for(auto it = str.begin(); it != str.end(); ++it) {
        char infer[10];
        snprintf(infer, 10, "%02x", (unsigned char)(*it));
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
    for(auto it = str.begin(); it != str.end(); ++it) {
        char c = *it;
        if(c == quote_mark || c == '\\') {
            // escape the character
            out << '\\' << c;

        } else if(!isprint(c)) { // character is not a printable ascii character
            // print the character as an escaped hexidecimal character constant
            char infer[10];
            snprintf(infer, 10, "%02x", (unsigned char)c);
            out << "\\x" << infer;
        } else {
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
