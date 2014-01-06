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
#include <string> // std::string
#include <vector> // std::vector
namespace dclass   // open namespace dclass
{

// A PrimeNumberGenerator this class generates a table of prime numbers, up to the limit of an int.
//     For a given integer n, it will return the nth prime number.  This will involve a recompute
//     step only if n is greater than any previous n.
class PrimeNumberGenerator
{
	public:
		PrimeNumberGenerator();

		// the indexing operator returns the nth prime number.  this[0] returns 2, this[1] returns 3;
		//     successively larger values of n return larger prime numbers, up to the largest prime
		//     number that can be represented in an int.
		unsigned int operator [](unsigned int n);

	private:
		std::vector<unsigned int> m_primes;
};

// A HashGenerator generates an arbitrary hash number from a sequence of ints.
class HashGenerator
{
	public:
		HashGenerator();

		// add_int adds another integer to the hash so far.
		void add_int(int num);

		// add_string adds a string to the hash, by breaking it down into a sequence of integers.
		void add_string(const std::string& str);

		uint32_t get_hash() const;

	private:
		uint32_t m_hash;
		unsigned int m_index;
		PrimeNumberGenerator m_primes;
};


} // close namespace dclass
