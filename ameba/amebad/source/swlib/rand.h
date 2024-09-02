/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

extern u32 rand_seed[4]; //z1, z2, z3, z4,
extern u32 rand_first;

u32 Rand(void);
u32 _rand(void);
int TRNG_get_random_bytes(void *dst, u32 size);
int TRNG_get_random_bytes_f_rng(void *p_rng, unsigned char *output, size_t output_size);
int RandBytes_Get(void *buf, u32 len);

