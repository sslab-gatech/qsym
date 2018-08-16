/*

Author: Debbie Nuttall <debbie@cromulence.co>

Copyright (c) 2014 Cromulence LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef PRNG_H
#define PRNG_H

#include <stdint.h>

void sprng(uint64_t seed);
uint64_t prng();
uint32_t random_in_range(uint32_t min, uint32_t max);

// Coefficients chosen from "An Experimental Exploration of
// Marsaglia's Xorshift Generators, Scrambled" by Sebastiano Vigna.
#define COEFFICIENT_A_64 12
#define COEFFICIENT_B_64 25
#define COEFFICIENT_C_64 27

#define COEFFICIENT_A_1024 27
#define COEFFICIENT_B_1024 13
#define COEFFICIENT_C_1024 46

// Multipliers chosen from "Tables of Linear Congruetial Generators of
// Different Sizes and Good Lattice Structure" by Pierre L'Ecuyer.
#define MULTIPLIER_1024 1865811235122147685LL
#define MULTIPLIER_64	1803442709493370165LL

#endif // PRNG_H