//     Function: Packer::parse_and_pack
//  Description: Parses an object's value according to the  file
//               syntax (e.g. as a default value string) and packs it.
//               Returns true on success, false on a parse error.
bool Packer::parse_and_pack(const std::string &formatted_object)
{
	std::istringstream strm(formatted_object);
	return parse_and_pack(strm);
}

//     Function: Packer::parse_and_pack
//  Description: Parses an object's value according to the  file
//               syntax (e.g. as a default value string) and packs it.
//               Returns true on success, false on a parse error.
bool Packer::parse_and_pack(std::istream &in)
{
	dc_init_parser_parameter_value(in, "parse_and_pack", *this);
	dcyyparse();
	dc_cleanup_parser();

	bool parse_error = (dc_error_count() != 0);
	if(parse_error)
	{
		_parse_error = true;
	}

	return !parse_error;
}

//     Function: Packer::unpack_literal_value
//  Description: Returns the literal string that represents the packed
//               value of the current field, and advances the field
//               pointer.
std::string Packer::unpack_literal_value()
{
	size_t start = _unpack_p;
	unpack_skip();
	assert(_unpack_p >= start);
	return std::string(_unpack_data + start, _unpack_p - start);
}