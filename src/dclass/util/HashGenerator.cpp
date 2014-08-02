// Filename: HashGenerator.cpp
// Created by: drose (22 Mar, 2001)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
// All use of this software is subject to the terms of the revised BSD license.
//

#include "HashGenerator.h"
#include "PrimeNumberGenerator.h"
namespace dclass   // open namespace dclass
{


// We multiply each consecutive integer by the next prime number and
// add it to the total.  This will generate pretty evenly-distributed
// hash numbers for an arbitrary sequence of ints.

// We do recycle the prime number table at some point, just to keep it
// from growing insanely large, however (and to avoid wasting time
// computing large prime numbers unnecessarily), and we also truncate
// the result to the low-order 32 bits.

#define MAX_PRIME_NUMBERS 10000
#define PRIMES PrimeNumberGenerator::singleton

// constructor
HashGenerator::HashGenerator() : m_hash(0), m_index(0)
{
}

// add_int adds another integer to the hash so far.
void HashGenerator::add_int(int num)
{
    m_hash += PRIMES[m_index] * num;
    m_index = (m_index + 1) % MAX_PRIME_NUMBERS;
}

// add_string adds a string to the hash, by breaking it down into a sequence of integers.
void HashGenerator::add_string(const std::string& str)
{
    add_int(str.length());
    for(auto it = str.begin(); it != str.end(); ++it) {
        add_int(*it);
    }
}

// get_hash returns the hash number generated.
uint32_t HashGenerator::get_hash() const
{
    return (uint32_t)(m_hash & 0xffffffff);
}


} // close namespace dclass
