// Filename: PrimeNumberGenerator.h
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
		int operator [](int n);

	private:
		std::vector<int> m_primes;
};


} // close namespace dclass
