// Filename: unpack.h
#pragma once

#include "../Element.h"
#include "../DataType.h"
#include <vector> // for std::vector
#include <string> // for std::string

namespace dclass   // open namespace dclass
{


// format outputs the packed data in a string formatted for a .dc file.
//     An Element represents any type with defined structure (Class, Field, method, etc...)
std::string format(const Element*, const std::vector<uint8_t> &packed);
void format(const Element*, const std::vector<uint8_t> &packed, std::ostream &out);

// format_hex outputs <str> as a hexidecimal constant enclosed in angle-brackets (<>).
std::string format_hex(const std::string &str);
void format_hex(const std::string &str, std::ostream &out);

// format_quoted outputs <str> enclosed in quotes after escaping the string.
//     Any instances of backslash (\) or the quoute character in the string are escaped.
//     Non-printable characters are replaced with an escaped hexidecimal constant.std::string format_quoted(const std::string &str);
std::string format_quoted(char quote_mark, const std::string &str);
void format_quoted(char quote_mark, const std::string &str, std::ostream &out);

// print outputs the packed data in a string formatted to be as human-friendly as possible.
//     An Element represents any type with defined structure (Class, Field, method, etc...)
std::string print(const Element*, const std::vector<uint8_t> &packed);
void print(const Element*, const std::vector<uint8_t> &packed, std::ostream &out);


class Unpacker
{
	public:
		Unpacker(const std::vector<uint8_t> &in, std::ostream &out);

		// format outputs the packed data in a string formatted for a .dc file.
		void format(const Element*);
		// print outputs the packed data in a string formatted to be as human-friendly as possible.
		void print(const Element*);

	private:
		const uint8_t* in;
		std::ostream &out;
		size_t offset;

		// format_as_number outputs the element to formatted numerically if it has a numeric type.
		//     Returns true and advances the offset if it is a number, or false otherwise.
		bool format_as_number(const Element*);

		//inline void check_offset();

		inline uint8_t unpack_uint8();
		inline uint16_t unpack_uint16();
		inline uint32_t unpack_uint32();
		inline uint64_t unpack_uint64();
		inline int8_t unpack_int8();
		inline int16_t unpack_int16();
		inline int32_t unpack_int32();
		inline int64_t unpack_int64();
		inline float unpack_float32();
		inline double unpack_float64();
		inline length_tag_t unpack_length();
		inline std::string unpack_char();
		inline std::string unpack_string(const Element*);
		inline std::string unpack_varstring();
		inline std::string unpack_blob(const Element*);
		inline std::string unpack_varblob();
};


} // close namespace dclass
