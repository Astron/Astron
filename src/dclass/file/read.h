// Filename: read.h
#pragma once
#include <iostream> // std::istream
#include <string>   // std::string
namespace dclass   // open namespace dclass
{


// Foward declarations
class File;

// append opens the given file or stream and parses it as a .dc file.  The distributed
//     classes defined in the file are added to the list of classes associated with the File.
//     When appending from a stream, a filename is optional only used to report errors.
bool append(File* f, std::istream &in, const std::string &filename);
bool append(File* f, const std::string &filename);

// read opens the given file or stream and parses it as a .dc file.  Classes defined in
//     the file are added to a new File object, and a pointer to that object is returned.
//     When reading from a stream, a filename is optional only used when reporting errors.
File* read(std::istream &in, const std::string &filename);
File* read(const std::string &filename);


} // close namespace dclass
