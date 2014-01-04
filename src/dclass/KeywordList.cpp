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
namespace dclass   // open namespace dclass
{


// empty list constructor
KeywordList::KeywordList()
{
}

// TODO: Evaluate whether this is being used
// copy constructor
KeywordList::KeywordList(const KeywordList& copy) :
	m_keywords(copy.m_keywords), m_keywords_by_name(copy.m_keywords_by_name)
{
}

// TODO: Evaluate whether this is being used
// copy assignment operator
void KeywordList::operator = (const KeywordList& copy)
{
	m_keywords = copy.m_keywords;
	m_keywords_by_name = copy.m_keywords_by_name;
}

// has_keyword returns true if this list includes the indicated keyword, false otherwise.
bool KeywordList::has_keyword(const std::string &name) const
{
	return (m_keywords_by_name.find(name) != m_keywords_by_name.end());
}

// get_num_keywords returns the number of keywords in the list.
size_t KeywordList::get_num_keywords() const
{
	return m_keywords.size();
}

// get_keyword returns the nth keyword in the list.
const Keyword* KeywordList::get_keyword(unsigned int n) const
{
	return m_keywords[n];
}

// get_keyword_by_name returns the keyword in the list with the indicated name,
//     or NULL if there is no keyword in the list with that name.
const Keyword* KeywordList::get_keyword_by_name(const std::string &name) const
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
bool KeywordList::compare_keywords(const KeywordList& other) const
{
	return m_keywords_by_name == other.m_keywords_by_name;
}

// copy_keywords replaces this keyword list with those from the other list.
void KeywordList::copy_keywords(const KeywordList& other)
{
	(*this) = other;
}

// add_keyword adds the indicated keyword to the list.
void KeywordList::add_keyword(const std::string& keyword)
{
	bool inserted = m_keywords_by_name.insert(keyword).second;
	if(inserted)
	{
		m_keywords.push_back(keyword);
	}

	return inserted;
}

// TODO: Evaluate whether this is being used
// clear_keywords removes all keywords from the field.
void KeywordList::clear_keywords()
{
	m_keywords.clear();
	m_keywords_by_name.clear();
}

// generate_hash accumulates the properties of these keywords into the hash.
void KeywordList::generate_hash(HashGenerator &hashgen) const
{
	hashgen.add_int(m_keywords_by_name.size());
	for(auto it = m_keywords_by_name.begin(); it != m_keywords_by_name.end(); ++it)
	{
		it->second->generate_hash(hashgen);
	}
}


} // close namespace dclass
