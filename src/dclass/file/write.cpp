// write opens the indicated filename for output and writes a parseable
//     description of all the known distributed classes to the file.
//     Returns true if the description is successfully written, false otherwise.
bool File::write(std::string filename, bool brief) const
{
	std::ofstream out;

	out.open(filename.c_str());

	if(!out)
	{
		std::cerr << "Can't open " << filename << " for output.\n";
		return false;
	}
	return write(out, brief);
}

// write writes a parseable description of all the known distributed classes to the stream.
//     Returns true if the description is successfully written, false otherwise.
bool File::write(std::ostream &out, bool brief) const
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
void KeywordList::output_keywords(std::ostream &out) const
{
	for(auto it = m_keywords.begin(); it != m_keywords.end(); ++it)
	{
		out << " " << (*it)->get_name();
	}
}