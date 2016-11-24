// Filename: write.cpp
#include "dc/DistributedType.h"

#include "write.h"
using namespace std;
namespace dclass   // open namespace dclass
{


ostream& indent(ostream& out, unsigned int indent_level)
{
    for(unsigned int i = 0; i < indent_level; i++) {
        out << ' ';
    }
    return out;
}

string format_type(unsigned int type)
{
    switch(type) {
    case T_CHAR:
        return "char";
    case T_INT8:
        return "int8";
    case T_INT16:
        return "int16";
    case T_INT32:
        return "int32";
    case T_INT64:
        return "int64";
    case T_UINT8:
        return "uint8";
    case T_UINT16:
        return "uint16";
    case T_UINT32:
        return "uint32";
    case T_UINT64:
        return "uint64";
    case T_FLOAT32:
        return "float32";
    case T_FLOAT64:
        return "float64";
    case T_STRING:
        return "string";
    case T_VARSTRING:
        return "varstring";
    case T_BLOB:
        return "blob";
    case T_VARBLOB:
        return "varblob";
    case T_ARRAY:
        return "array";
    case T_VARARRAY:
        return "vararray";
    case T_STRUCT:
        return "struct";
    case T_METHOD:
        return "method";
    case T_INVALID:
        return "invalid";
    default:
        return "error";
    }
}


} // close namespace dclass

/*
// write opens the indicated filename for output and writes a parseable
//     description of all the known distributed classes to the file.
//     Returns true if the description is successfully written, false otherwise.
bool File::write(string filename, bool brief) const
{
    ofstream out;

    out.open(filename.c_str());

    if(!out)
    {
        cerr << "Can't open " << filename << " for output.\n";
        return false;
    }
    return write(out, brief);
}

// write writes a parseable description of all the known distributed classes to the stream.
//     Returns true if the description is successfully written, false otherwise.
bool File::write(ostream &out, bool brief) const
{
    if(!m_imports.empty())
    {
        for(auto imp_it = m_imports.begin(); imp_it != m_imports.end(); ++imp_it)
        {
            const Import &import = (*imp_it);
            if(import.m_symbols.empty())
            {
                out << "import " << import.m_module << "\n";
            }
            else
            {
                out << "from " << import.m_module << " import ";
                auto sym_it = import.m_symbols.begin();
                out << *sym_it;
                ++sym_it;
                while(sym_it != import.m_symbols.end())
                {
                    out << ", " << *sym_it;
                    ++sym_it;
                }
                out << "\n";
            }
        }
        out << "\n";
    }


    for(auto it = m_declarations.begin(); it != m_declarations.end(); ++it)
    {
        (*it)->write(out, brief, 0);
        out << "\n";
    }

    return !out.fail();
}

// output_keywords writes the keywords in the list to the output stream.
void KeywordList::output_keywords(ostream &out) const
{
    for(auto it = m_keywords.begin(); it != m_keywords.end(); ++it)
    {
        out << " " << (*it)->get_name();
    }
}


// output writes a string representation of this instance to <out>.
void Keyword::output(ostream &out, bool brief) const
{
    out << "keyword " << m_name;
}

// write writes a string representation of this instance to <out>.
void Keyword::write(ostream &out, bool, int indent_level) const
{
    indent(out, indent_level) << "keyword " << m_name << ";\n";
}

// output writes a representation of the parameter to an output stream
void Parameter::output(ostream &out, bool brief) const
{
    string name;
    if(!brief)
    {
        name = get_name();
    }
    output_instance(out, brief, "", name, "");
}

// write writes a representation of the parameter to an output stream
void Parameter::write(ostream &out, bool brief, int indent_level) const
{
    // we must always output the name when the parameter occurs by
    // itself within a class, so we pass get_name() even if brief is
    // true.
    write_instance(out, brief, indent_level, "", get_name(), "");
}

// write_instance formats the parameter in the C++-like dc syntax as a typename and identifier.
void Parameter::write_instance(ostream &out, bool brief, int indent_level,
                               const string &prename, const string &name,
                               const string &postname) const
{
    indent(out, indent_level);
    output_instance(out, brief, prename, name, postname);
    output_keywords(out);
    out << ";";
    if(!brief && m_id >= 0)
    {
        out << "  // field " << m_id;
    }
    out << "\n";
}

// output_typedef_name formats the instance like output_instance, but uses the typedef name instead.
void Parameter::output_typedef_name(ostream &out, bool, const string &prename,
                                    const string &name, const string &postname) const
{
    out << get_typedef()->get_name();
    if(!prename.empty() || !name.empty() || !postname.empty())
    {
        out << " " << prename << name << postname;
    }
}

// write_typedef_name formats the instance like write_instance, but uses the typedef name instead.
void Parameter::write_typedef_name(ostream &out, bool brief, int indent_level,
                                   const string &prename, const string &name,
                                   const string &postname) const
{
    indent(out, indent_level) << get_typedef()->get_name();
    if(!prename.empty() || !name.empty() || !postname.empty())
    {
        out << " " << prename << name << postname;
    }
    output_keywords(out);
    out << ";";
    if(!brief && m_id >= 0)
    {
        out << "  // field " << m_id;
    }
    out << "\n";
}


// output_instance formats the parameter in .dc syntax as a typename and identifier.
void SimpleParameter::output_instance(ostream &out, bool brief, const string &prename,
                                      const string &name, const string &postname) const
{
    if(get_typedef() != nullptr)
    {
        output_typedef_name(out, brief, prename, name, postname);

    }
    else
    {
        out << m_datatype;
        if(m_has_modulus)
        {
            out << "%" << m_orig_modulus;
        }
        if(m_divisor != 1)
        {
            out << "/" << m_divisor;
        }

        // TODO: Replace
        switch(m_datatype)
        {
            case DT_int8:
            case DT_int16:
            case DT_int32:
                if(!m_int_range.is_empty())
                {
                    out << "(";
                    m_int_range.output(out, m_divisor);
                    out << ")";
                }
                break;

            case DT_int64:
                if(!m_int64_range.is_empty())
                {
                    out << "(";
                    m_int64_range.output(out, m_divisor);
                    out << ")";
                }
                break;

            case DT_uint8:
            case DT_uint16:
            case DT_uint32:
                if(!m_uint_range.is_empty())
                {
                    out << "(";
                    m_uint_range.output(out, m_divisor);
                    out << ")";
                }
                break;

            case DT_char:
                if(!m_uint_range.is_empty())
                {
                    out << "(";
                    m_uint_range.output_char(out, m_divisor);
                    out << ")";
                }
                break;

            case DT_uint64:
                if(!m_uint64_range.is_empty())
                {
                    out << "(";
                    m_uint64_range.output(out, m_divisor);
                    out << ")";
                }
                break;

            case DT_float64:
                if(!m_double_range.is_empty())
                {
                    out << "(";
                    m_double_range.output(out, m_divisor);
                    out << ")";
                }
                break;

            case DT_string:
                if(!m_uint_range.is_empty())
                {
                    out << "(";
                    m_uint_range.output(out, m_divisor);
                    out << ")";
                }
                break;
            default:
                break;
        }

        if(!prename.empty() || !name.empty() || !postname.empty())
        {
            out << " " << prename << name << postname;
        }
    }
}

// output_instance formats the parameter to the syntax of an array parameter in a .dc file
//     as TYPE IDENTIFIER[RANGE] with optional IDENTIFIER and RANGE,
//     and outputs the formatted string to the stream.
void ArrayParameter::output_instance(ostream &out, bool brief, const string &prename,
                                     const string &name, const string &postname) const
{
    if(get_typedef() != nullptr)
    {
        output_typedef_name(out, brief, prename, name, postname);

    }
    else
    {
        ostringstream strm;

        strm << "[";
        // TODO: fix
        //m_array_size_range.output(strm);
        strm << "]";

        m_element_type->output_instance(out, brief, prename, name,
                                        postname + strm.str());
    }
}

// output formats a string representation of the class in .dc file syntax
//     as "dclass IDENTIFIER" or "struct IDENTIFIER" with structs having optional IDENTIFIER,
//     and outputs the formatted string to the stream.
void Struct::output(ostream &out) const
{
    if(this->as_class())
    {
        out << "dclass";
    }
    else
    {
        out << "struct";
    }
    if(!m_name.empty())
    {
        out << " " << m_name;
    }
}

// output formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Struct::output(ostream &out, bool brief) const
{
    output_instance(out, brief, "", "", "");
}

// write formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Struct::write(ostream &out, bool brief, int indent_level) const
{
    indent(out, indent_level);
    if(this->as_class())
    {
        out << "dclass";
    }
    else
    {
        out << "struct";
    }
    if(!m_name.empty())
    {
        out << " " << m_name;
    }

    // TODO: Move and update class write
    if(!m_parents.empty())
    {
        auto it = m_parents.begin();
        out << " : " << (*it)->m_name;
        ++it;
        while(it != m_parents.end())
        {
            out << ", " << (*it)->m_name;
            ++it;
        }
    }


    out << " {";
    if(!brief && m_id >= 0)
    {
        out << "  // index " << m_id;
    }
    out << "\n";

    // TODO: Move and update class writing
    if(m_constructor != nullptr)
    {
        m_constructor->write(out, brief, indent_level + 2);
    }


    for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
    {
        (*it)->write(out, brief, indent_level + 2);


        //if (true || (*fi)->has_default_value()) {
        //  indent(out, indent_level + 2) << "// = ";
        //  Packer packer;
        //  packer.set_unpack_data((*fi)->get_default_value());
        //  packer.begin_unpack(*fi);
        //  packer.unpack_and_format(out, false);
        //  if (!packer.end_unpack()) {
        //    out << "<error>";
        //  }
        //  out << "\n";
        //}
    }

    indent(out, indent_level) << "};\n";
}

// output_instance formats a string representation of the class in .dc file syntax
//     as dclass IDENTIFIER : SUPERCLASS, ... {FIELD, ...}; with optional SUPERCLASSES and FIELDS,
//     and outputs the formatted string to the stream.
void Struct::output_instance(ostream &out, bool brief, const string &prename,
                            const string &name, const string &postname) const
{
    if(this->as_class())
    {
        out << "dclass";
    }
    else
    {
        out << "struct";
    }
    if(!m_name.empty())
    {
        out << " " << m_name;
    }

    if(!m_parents.empty())
    {
        auto it = m_parents.begin();
        out << " : " << (*it)->m_name;
        ++it;
        while(it != m_parents.end())
        {
            out << ", " << (*it)->m_name;
            ++it;
        }
    }


    out << " {";

    if(m_constructor != nullptr)
    {
        m_constructor->output(out, brief);
        out << "; ";
    }


    for(auto it = m_fields.begin(); it != m_fields.end(); ++it)
    {
        (*it)->output(out, brief);
        out << "; ";
    }

    out << "}";
    if(!prename.empty() || !name.empty() || !postname.empty())
    {
        out << " " << prename << name << postname;
    }
}

// output formats the field to the syntax of an atomic field in a .dc file
//     as IDENTIFIER(ELEMENTS, ...) KEYWORDS with optional ELEMENTS and KEYWORDS,
//     and outputs the formatted string to the stream.
void AtomicField::output(ostream &out, bool brief) const
{
    out << m_name << "(";

    if(!m_elements.empty())
    {
        auto it = m_elements.begin();
        output_element(out, brief, *it);
        ++it;
        while(it != m_elements.end())
        {
            out << ", ";
            output_element(out, brief, *it);
            ++it;
        }
    }
    out << ")";

    output_keywords(out);
}

// write generates a parseable description of the object to the indicated output stream.
void AtomicField::write(ostream &out, bool brief, int indent_level) const
{
    indent(out, indent_level);
    output(out, brief);
    out << ";";
    if(!brief && m_id >= 0)
    {
        out << "  // field " << m_id;
    }
    out << "\n";
}

// output_element formats a parameter as an element for output into .dc file syntax.
//     Used internally by AtomicField's output() method.
void AtomicField::output_element(ostream &out, bool brief, Parameter *element) const
{
    element->output(out, brief);

    if(!brief && element->has_default_value())
    {
        out << " = ";
        format_value(element, element->get_default_value(), out);
    }
}

void AtomicField::refresh_default_value()
{
    m_default_value = string();
    for(auto it = m_elements.begin(); it != m_elements.end(); ++it)
    {
        m_default_value += (*it)->get_default_value();
    }
    m_default_value_stale = false;
}

// output writes a representation of the field to an output stream
void MolecularField::output(ostream &out, bool brief) const
{
    out << m_name;

    if(!m_fields.empty())
    {
        auto field_it = m_fields.begin();
        out << " : " << (*field_it)->get_name();
        ++field_it;
        while(field_it != m_fields.end())
        {
            out << ", " << (*field_it)->get_name();
            ++field_it;
        }
    }

    out << ";";
}

// write writes a representation of the field to an output stream
void MolecularField::write(ostream &out, bool brief, int indent_level) const
{
    indent(out, indent_level);
    output(out, brief);
    if(!brief)
    {
        out << "  // field " << m_id;
    }
    out << "\n";
}
*/
