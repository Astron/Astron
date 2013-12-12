// Filename: HashGenerator.h
// Created by: drose (22 Mar, 2001)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#pragma once
#include "dcbase.h"
#include "PrimeNumberGenerator.h"
namespace dclass   // open namespace dclass
{


// A HashGenerator generates an arbitrary hash number from a sequence of ints.
class HashGenerator
{
	public:
		HashGenerator();

		// add_int adds another integer to the hash so far.
		void add_int(int num);

		// add_string adds a string to the hash, by breaking it down into a sequence of integers.
		void add_string(const string &str);

		uint32_t get_hash() const;

	private:
		int32_t m_hash;
		int m_index;
		PrimeNumberGenerator m_primes;
};


} // close namespace dclass
