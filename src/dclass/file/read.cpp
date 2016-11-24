// Filename: read.cpp
#include <fstream> // std::ifstream
#include "dc/File.h"
#include "parserDefs.h"

#include "read.h"
using namespace std;
namespace dclass   // open namespace dclass
{


// append opens the given file or stream and parses it as a .dc file.  The distributed
//     classes defined in the file are added to the list of classes associated with the File.
//     When appending from a stream, a filename is optional only used to report errors.
bool append(File* f, istream &in, const string &filename)
{
    init_file_parser(in, filename, *f);
    run_parser();
    cleanup_parser();
    return (parser_error_count() == 0);
}
bool append(File* f, const string &filename)
{
    ifstream in;
    in.open(filename.c_str());
    if(!in) {
        cerr << "Cannot open " << filename << " for reading.\n";
        return false;
    }
    return append(f, in, filename);
}

// read opens the given file or stream and parses it as a .dc file.  Classes defined in
//     the file are added to a new File object, and a pointer to that object is returned.
//     When reading from a stream, a filename is optional only used when reporting errors.
File* read(istream &in, const string &filename)
{
    File* f = new File();
    bool ok = append(f, in, filename);
    if(ok) {
        return f;
    }

    return nullptr;
}
File* read(const string &filename)
{
    File* f = new File();
    bool ok = append(f, filename);
    if(ok) {
        return f;
    }

    return nullptr;
}


} // close namespace dclass
