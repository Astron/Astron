// Filename: PrimeNumberGenerator.cpp
// Created by: drose (22 Mar, 2001)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
// All use of this software is subject to the terms of the revised BSD license.
//

#include "PrimeNumberGenerator.h"
namespace dclass   // open namespace dclass
{

PrimeNumberGenerator PrimeNumberGenerator::singleton = PrimeNumberGenerator();

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
    while(m_primes.size() <= n) {
        // Is candidate prime?  It is not if any one of the already-found
        // prime numbers (up to its square root) divides it evenly.
        bool maybe_prime = true;
        unsigned int j = 0;
        while(maybe_prime && m_primes[j] * m_primes[j] <= candidate) {
            if((m_primes[j] * (candidate / m_primes[j])) == candidate) {
                // This one is not prime.
                maybe_prime = false;
            }
            j++;
        }
        if(maybe_prime) {
            // Hey, we found a prime!
            m_primes.push_back(candidate);
        }
        candidate++;
    }

    return m_primes[n];
}


} // close namespace dclass
