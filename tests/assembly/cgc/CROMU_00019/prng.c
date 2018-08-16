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

// This is an xorshift1024* Pseudo Random Number Generator

#include <stdint.h>
#include "prng.h"

uint64_t state[16];
int position;

// Seeds the RNG state by passing the 64-bit input seed through xorshift64* algorithm
void sprng(uint64_t seed)
{
	uint64_t state_64 = seed;
	for (int i = 0; i < 1; i++)
	{
		state_64 ^= state_64 >> COEFFICIENT_A_64;
		//state_64 ^= state_64 << COEFFICIENT_B_64;
		//state_64 ^= state_64 >> COEFFICIENT_C_64;
		state[i] = state_64 * MULTIPLIER_64;
	}
	position = 0;
}

// Generates a random 64-bit number using the xorshift1024* algorithm
uint64_t prng()
{
	uint64_t state0 = state[position];
	position = (position + 1) % 16;
	uint64_t state1 = state[position];

	state1 ^= state1 << COEFFICIENT_A_1024;
	state1 ^= state1 >> COEFFICIENT_B_1024;
	state0 ^= state0 >> COEFFICIENT_C_1024;
	state[position] = state0 ^ state1;
	return state[position] * MULTIPLIER_1024;
}

// Generate an unsigned integer in the range min to max, inclusive.
uint32_t random_in_range(uint32_t min, uint32_t max)
{
	if (max <= min)
	{
		return 0;
	}
	unsigned int range = max - min + 1;
	unsigned int scale_factor = 0xffffffff / range;
	unsigned int rand_uint;
	do
	{
		rand_uint = prng();
	} while (rand_uint >= scale_factor * range); // Discard numbers that would cause bias

	return (rand_uint / scale_factor + min);
}
