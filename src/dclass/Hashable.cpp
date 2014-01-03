// Filename: HashGenerator.cpp
// Created by: drose (22 Mar, 2001)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//

#include "Hashable.h"
namespace dclass   // open namespace dclass
{


/* PrimeNumberGenerator */

// constructor
PrimeNumberGenerator::PrimeNumberGenerator()
{
	m_primes.push_back(2);
}

// the indexing operator returns the nth prime number.  this[0] returns 2, this[1] returns 3;
//     successively larger values of n return larger prime numbers, up to the largest prime
//     number that can be represented in an int.
unsigned int PrimeNumberGenerator::operator [](unsigned int n)
{
	// Compute the prime numbers between the last-computed prime number and n.
	unsigned int candidate = m_primes.back() + 1;
	while(m_primes.size() <= n)
	{
		// Is candidate prime?  It is not if any one of the already-found
		// prime numbers (up to its square root) divides it evenly.
		bool maybe_prime = true;
		unsigned int j = 0;
		while(maybe_prime && m_primes[j] * m_primes[j] <= candidate)
		{
			if((m_primes[j] * (candidate / m_primes[j])) == candidate)
			{
				// This one is not prime.
				maybe_prime = false;
			}
			j++;
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


/* HashGenerator */

// We multiply each consecutive integer by the next prime number and
// add it to the total.  This will generate pretty evenly-distributed
// hash numbers for an arbitrary sequence of ints.

// We do recycle the prime number table at some point, just to keep it
// from growing insanely large, however (and to avoid wasting time
// computing large prime numbers unnecessarily), and we also truncate
// the result to the low-order 32 bits.

#define MAX_PRIME_NUMBERS 10000

// constructor
HashGenerator::HashGenerator() : m_hash(0), m_index(0)
{
}

// add_int adds another integer to the hash so far.
void HashGenerator::add_int(int num)
{
	m_hash += m_primes[m_index] * num;
	m_index = (m_index + 1) % MAX_PRIME_NUMBERS;
}

// add_string adds a string to the hash, by breaking it down into a sequence of integers.
void HashGenerator::add_string(const std::string &str)
{
	add_int(str.length());
	for(auto it = str.begin(); it != str.end(); ++it)
	{
		add_int(*it);
	}
}

// get_hash returns the hash number generated.
uint32_t HashGenerator::get_hash() const
{
	return (uint32_t)(m_hash & 0xffffffff);
}


} // close namespace dclass
