
PackerInterface::PackerInterface(const std::string &name) : m_name(name)
{
	m_has_fixed_byte_size = false;
	m_fixed_byte_size = 0;
	m_has_range_limits = false;
	m_num_length_bytes = 0;
	m_has_nested_fields = false;
	m_num_nested_fields = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: PackerInterface::check_match
//       Access: Published
//  Description: Returns true if this interface is bitwise the same as
//               the interface described with the indicated formatted
//               string, e.g. "(uint8, uint8, int16)", or false
//               otherwise.
//
//               If File is not NULL, it specifies the File that
//               was previously loaded, from which some predefined
//               structs and typedefs may be referenced in the
//               description string.
////////////////////////////////////////////////////////////////////
bool PackerInterface::
check_match(const std::string &description, File *dcfile) const
{
	bool match = false;

	std::istringstream strm(description);
	dc_init_parser_parameter_description(strm, "check_match", dcfile);
	dcyyparse();
	dc_cleanup_parser();

	Field *field = dc_get_parameter_description();
	if(field != NULL)
	{
		match = check_match(field);
		delete field;
	}

	if(dc_error_count() == 0)
	{
		return match;
	}

	// Parse error: no match is allowed.
	return false;
}

} // close namespace dclass
