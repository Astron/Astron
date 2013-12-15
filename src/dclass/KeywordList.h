// Filename: KeywordList.h
// Created by: drose (25 Jul, 2005)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include <map> // for std::map
#include <vector> // for std::vector
namespace dclass   // open namespace dclass
{


// Forward declaration
class Keyword;
class HashGenerator;

// KeywordList this is a list of keywords (see Keyword) that may be set on a particular field.
class KeywordList
{
	public:
		// empty list constructor
		KeywordList();
		// copy constructor
		KeywordList(const KeywordList &copy);
		// copy assignment operator
		void operator = (const KeywordList &copy);
		// destructor
		~KeywordList();

		// has_keyword returns true if this list includes the indicated keyword, false otherwise.
		bool has_keyword(const std::string &name) const;
		// has_keyword returns true if this list includes the indicated keyword, false otherwise.
		bool has_keyword(const Keyword *keyword) const;
		// get_num_keywords returns the number of keywords in the list.
		int get_num_keywords() const;
		// get_keyword returns the nth keyword in the list.
		const Keyword *get_keyword(int n) const;
		// get_keyword_by_name returns the requested keyword if it exists.
		const Keyword *get_keyword_by_name(const std::string &name) const;

		// compare_keywords returns true if this list has the same keywords as the other list,
		//     false if some keywords differ. Order is not considered important.
		bool compare_keywords(const KeywordList &other) const;

		// copy_keywords replaces this keyword list with those from the other list.
		void copy_keywords(const KeywordList &other);

		// add_keyword adds the indicated keyword to the list.
		//     Returns true if it is added, false if it was already there.
		bool add_keyword(const Keyword *keyword);

		// clear_keywords removes all keywords from the field.
		void clear_keywords();

		// output_keywords writes the keywords in the list to the output stream.
		void output_keywords(std::ostream &out) const;

		// generate_hash accumulates the properties of these keywords into the hash.
		void generate_hash(HashGenerator &hashgen) const;

	private:
		std::vector<const Keyword*> m_keywords; // the actual list of keywords
		std::map<std::string, const Keyword*> m_keywords_by_name; // a map of name to keywords in list

		int m_flags;
};


} // close namespace dclass
