// Filename: read.h
#pragma once

#include "ParserDefs.h"
#include "LexerDefs.h"
#include "../File.h"

namespace dclass   // open namespace dclass
{


// append opens the given file or stream and parses it as a .dc file.  The distributed
//     classes defined in the file are added to the list of classes associated with the File.
//     When appending from a stream, a filename is optional only used to report errors.
bool append(File* f, std::istream &in, const std::string &filename)
{
	dc_init_parser(in, filename, *f);
	dcyyparse();
	dc_cleanup_parser();

	return (dc_error_count() == 0);
}
bool append(File* f, const std::string &filename)
{
	std::ifstream in;
	in.open(filename.c_str());
	if(!in)
	{
		std::cerr << "Cannot open " << filename << " for reading.\n";
		return false;
	}
	return append(f, in, filename);
}


// read opens the given file or stream and parses it as a .dc file.  Classes defined in
//     the file are added to a new File object, and a pointer to that object is returned.
//     When reading from a stream, a filename is optional only used when reporting errors.
const File* read(std::istream &in, const std::string &filename)
{
	File* f = new File();
	bool ok = append(f, in, filename);
	if(ok)
	{
		return f;
	}

	return NULL;
}
const File* read(const std::string &filename)
{
	File* f = new File();
	bool ok = append(f, filename);
	if(ok)
	{
		return f;
	}

	return NULL;
}


} // close namespace dclass
