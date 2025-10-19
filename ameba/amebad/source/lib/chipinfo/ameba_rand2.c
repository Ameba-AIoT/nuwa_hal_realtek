/*
 * Copyright (c) 2024 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "basic_types.h"
#include "ameba_soc.h"

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define __SYSTEM_CLOCK    CONFIG_PLATFORM_CLOCK

unsigned int rand_x = 123456789;

u32 Rand2(void)
{
	static unsigned int y = 362436;
	static unsigned int z = 521288629;
	static unsigned int c = 7654321;

	unsigned long long t, a = 698769069;

	rand_x = 69069 * rand_x + 12345;
	y ^= (y << 13);
	y ^= (y >> 17);
	y ^= (y << 5);
	t = a * z + c;
	c = (t >> 32);
	z = t;

	return rand_x + y + z;
}

u32 Rand3(u32 seed)
{
	static unsigned int y = 362436;
	static unsigned int z = 521288629;
	static unsigned int c = 7654321;

	unsigned long long t, a = 698769069;

	rand_x = 69069 * rand_x + 12345 + seed;
	y ^= (y << 13);
	y ^= (y >> 17);
	y ^= (y << 5);
	t = a * z + c;
	c = (t >> 32);
	z = t;

	return rand_x + y + z;
}

u32 _rand(void)
{
	return Rand();
}

u32 __wrap_rand(void)
{
	return Rand();
}

int TRNG_get_random_bytes(void *dst, u32 size)
{
	return RandBytes_Get(dst, size);
}

int TRNG_get_random_bytes_f_rng(void *p_rng, unsigned char *output, size_t output_size)
{
	UNUSED(p_rng);
	return TRNG_get_random_bytes(output, output_size);
}
