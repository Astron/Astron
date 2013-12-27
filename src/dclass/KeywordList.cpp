// Filename: KeywordList.cpp
// Created by: drose (25 Jul, 2005)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "KeywordList.h"
#include "Keyword.h"
#include "HashGenerator.h"
#include <assert.h>
namespace dclass   // open namespace dclass
{


// empty list constructor
KeywordList::KeywordList() : m_flags(0)
{
}

// copy constructor
KeywordList::KeywordList(const KeywordList &copy) : m_keywords(copy.m_keywords),
	m_keywords_by_name(copy.m_keywords_by_name), m_flags(copy.m_flags)
{
}

// copy assignment operator
void KeywordList::operator = (const KeywordList &copy)
{
	m_keywords = copy.m_keywords;
	m_keywords_by_name = copy.m_keywords_by_name;
	m_flags = copy.m_flags;
}

// destructor
KeywordList::~KeywordList()
{
	assert(m_keywords_by_name.size() == m_keywords.size());
}

// has_keyword returns true if this list includes the indicated keyword, false otherwise.
bool KeywordList::has_keyword(const std::string &name) const
{
	return (m_keywords_by_name.find(name) != m_keywords_by_name.end());
}

// has_keyword returns true if this list includes the indicated keyword, false otherwise.
bool KeywordList::has_keyword(const Keyword *keyword) const
{
	return has_keyword(keyword->get_name());
}

// get_num_keywords returns the number of keywords in the list.
int KeywordList::get_num_keywords() const
{
	assert(m_keywords_by_name.size() == m_keywords.size());
	return m_keywords.size();
}

// get_keyword returns the nth keyword in the list.
const Keyword *KeywordList::get_keyword(int n) const
{
	assert(n >= 0 && n < (int)m_keywords.size());
	return m_keywords[n];
}

// get_keyword_by_name returns the keyword in the list with the indicated name,
//     or NULL if there is no keyword in the list with that name.
const Keyword *KeywordList::get_keyword_by_name(const std::string &name) const
{
	auto key_it = m_keywords_by_name.find(name);
	if(key_it != m_keywords_by_name.end())
	{
		return key_it->second;
	}

	return NULL;
}

// compare_keywords returns true if this list has the same keywords as the other list,
//     false if some keywords differ. Order is not considered important.
bool KeywordList::compare_keywords(const KeywordList &other) const
{
	return m_keywords_by_name == other.m_keywords_by_name;
}

// copy_keywords replaces this keyword list with those from the other list.
void KeywordList::copy_keywords(const KeywordList &other)
{
	(*this) = other;
}

// add_keyword adds the indicated keyword to the list.
//     Returns true if it is added, false if it was already there.
bool KeywordList::add_keyword(const Keyword *keyword)
{
	bool inserted = m_keywords_by_name.insert(
		std::map<std::string, const Keyword*>::value_type(keyword->get_name(), keyword)).second;
	if(inserted)
	{
		m_keywords.push_back(keyword);
		m_flags |= keyword->get_historical_flag();
	}

	return inserted;
}

// clear_keywords removes all keywords from the field.
void KeywordList::clear_keywords()
{
	m_keywords.clear();
	m_keywords_by_name.clear();
	m_flags = 0;
}

// output_keywords writes the keywords in the list to the output stream.
void KeywordList::output_keywords(std::ostream &out) const
{
	for(auto it = m_keywords.begin(); it != m_keywords.end(); ++it)
	{
		out << " " << (*it)->get_name();
	}
}

// generate_hash accumulates the properties of these keywords into the hash.
void KeywordList::generate_hash(HashGenerator &hashgen) const
{
	if(m_flags != ~0)
	{
		// All of the flags are historical flags only, so add just the
		// flags bitmask to keep the hash code the same as it has
		// historically been.
		hashgen.add_int(m_flags);

	}
	else
	{
		// There is at least one custom flag, so go ahead and make the
		// hash code reflect it.

		hashgen.add_int(m_keywords_by_name.size());
		for(auto it = m_keywords_by_name.begin(); it != m_keywords_by_name.end(); ++it)
		{
			it->second->generate_hash(hashgen);
		}
	}
}


} // close namespace dclass
