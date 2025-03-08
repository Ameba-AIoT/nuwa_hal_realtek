/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_power_f32.c
 * Description:  Sum of the squares of the elements of a floating-point vector
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "arm_math.h"

/**
 * @ingroup groupStats
 */

/**
 * @defgroup power Power
 *
 * Calculates the sum of the squares of the elements in the input vector.
 * The underlying algorithm is used:
 *
 * <pre>
 * 	Result = pSrc[0] * pSrc[0] + pSrc[1] * pSrc[1] + pSrc[2] * pSrc[2] + ... + pSrc[blockSize-1] * pSrc[blockSize-1];
 * </pre>
 *
 * There are separate functions for floating point, Q31, Q15, and Q7 data types.
 */

/**
 * @addtogroup power
 * @{
 */


/**
 * @brief Sum of the squares of the elements of a floating-point vector.
 * @param[in]       *pSrc points to the input vector
 * @param[in]       blockSize length of the input vector
 * @param[out]      *pResult sum of the squares value returned here
 * @return none.
 *
 */


void arm_power_f32(
	float32_t *pSrc,
	uint32_t blockSize,
	float32_t *pResult)
{
	float32_t sum = 0.0f;                          /* accumulator */
	float32_t in;                                  /* Temporary variable to store input value */
	uint32_t blkCnt;                               /* loop counter */

#if defined (ARM_MATH_DSP)
	/* Run the below code for Cortex-M4 and Cortex-M3 */

	/*loop Unrolling */
	blkCnt = blockSize >> 2u;

	/* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
	 ** a second loop below computes the remaining 1 to 3 samples. */
	while (blkCnt > 0u) {
		/* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
		/* Compute Power and then store the result in a temporary variable, sum. */
		in = *pSrc++;
		sum += in * in;
		in = *pSrc++;
		sum += in * in;
		in = *pSrc++;
		sum += in * in;
		in = *pSrc++;
		sum += in * in;

		/* Decrement the loop counter */
		blkCnt--;
	}

	/* If the blockSize is not a multiple of 4, compute any remaining output samples here.
	 ** No loop unrolling is used. */
	blkCnt = blockSize % 0x4u;


#else
	/* Run the below code for Cortex-M0 */

	/* Loop over blockSize number of values */
	blkCnt = blockSize;

#endif /* #if defined (ARM_MATH_DSP) */


	while (blkCnt > 0u) {
		/* C = A[0] * A[0] + A[1] * A[1] + A[2] * A[2] + ... + A[blockSize-1] * A[blockSize-1] */
		/* compute power and then store the result in a temporary variable, sum. */
		in = *pSrc++;
		sum += in * in;

		/* Decrement the loop counter */
		blkCnt--;
	}

	/* Store the result to the destination */
	*pResult = sum;
}

/**
 * @} end of power group
 */
