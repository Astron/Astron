// Filename: PrimeNumberGenerator.cpp
// Created by: drose (22 March, 2001)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "PrimeNumberGenerator.h"
namespace dclass   // open namespace dclass
{


// constructor
PrimeNumberGenerator::PrimeNumberGenerator()
{
	m_primes.push_back(2);
}

// the indexing operator returns the nth prime number.  this[0] returns 2, this[1] returns 3;
//     successively larger values of n return larger prime numbers, up to the largest prime
//     number that can be represented in an int.
int PrimeNumberGenerator::operator [](int n)
{
	assert(n >= 0, 0);

	// Compute the prime numbers between the last-computed prime number
	// and n.
	int candidate = m_primes.back() + 1;
	while((int)m_primes.size() <= n)
	{
		// Is candidate prime?  It is not if any one of the already-found
		// prime numbers (up to its square root) divides it evenly.
		bool maybe_prime = true;
		int j = 0;
		while(maybe_prime && m_primes[j] * m_primes[j] <= candidate)
		{
			if((m_primes[j] * (candidate / m_primes[j])) == candidate)
			{
				// This one is not prime.
				maybe_prime = false;
			}
			j++;
			assert(j < (int)m_primes.size(), 0);
		}
		if(maybe_prime)
		{
			// Hey, we found a prime!
			m_primes.push_back(candidate);
		}
		candidate++;
	}

	return m_primes[n];
}


} // close namespace dclass
