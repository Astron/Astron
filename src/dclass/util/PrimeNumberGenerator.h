// Filename: PrimeNumberGenerator.h
// Created by: drose (22 Mar, 2001)
//
// Copyright (c) Carnegie Mellon University.  All rights reserved.
// All use of this software is subject to the terms of the revised BSD license.
//

#pragma once
#include <vector> // std::vector
namespace dclass   // open namespace dclass
{

// A PrimeNumberGenerator this class generates a table of prime numbers, up to the limit of an int.
//     For a given integer n, it will return the nth prime number.  This will involve a recompute
//     step only if n is greater than any previous n.
class PrimeNumberGenerator
{
  public:
    static PrimeNumberGenerator singleton; // why would we ever need more than one?

    // the indexing operator returns the nth prime number.  this[0] returns 2, this[1] returns 3;
    //     successively larger values of n return larger prime numbers, up to the largest prime
    //     number that can be represented in an int.
    unsigned int operator [](unsigned int n);

  private:
    PrimeNumberGenerator();
    std::vector<unsigned int> m_primes;
};


} // close namespace dclass
