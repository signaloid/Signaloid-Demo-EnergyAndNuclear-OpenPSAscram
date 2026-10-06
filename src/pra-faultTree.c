/*
 *	Copyright (c) 2026, Signaloid.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

/*
 *	pra-faultTree.c — Gate-by-gate fault tree probability evaluation.
 *
 *	Implements the standard recursive gate evaluation algorithm from:
 *	  - Vesely et al. (1981), NUREG-0492, "Fault Tree Handbook", Section 5.2.
 *	  - Kumamoto & Henley (1996), Chapter 4.
 *
 *	This is exact for tree-structured fault trees with independent basic
 *	events.  All operations are pure arithmetic with no data-dependent
 *	branches on probability values, making them compatible with Signaloid
 *	UxHw distributional propagation.
 */

#include "pra-faultTree.h"

/**
 *	@brief	Evaluate the K-of-N gate using combinatorial enumeration.
 *
 *	Computes P(at least K of N children fail) by summing over all C(N,K)
 *	subsets, using inclusion-exclusion to avoid double-counting.
 *
 *	For small N (as in PRA fault trees), this is computed by iterating
 *	over all 2^N subsets via bitmask enumeration.  This approach is
 *	tractable for N <= 8 (our maximum) and produces exact results.
 *
 *	Reference: NUREG-0492, Section 3.3.4 (Voting gates).
 *
 *	@param	childProbabilities	: Array of child failure probabilities.
 *	@param	numberOfChildren	: Number of children (N).
 *	@param	kRequired		: Minimum number of failures required (K).
 *	@return	double			: Probability that at least K of N children fail.
 */
static double
praEvaluateKOfNGate(
	double *    childProbabilities,
	size_t      numberOfChildren,
	size_t      kRequired)
{
	double  result          = 0.0;
	size_t  totalSubsets    = (size_t) 1 << numberOfChildren;

	for (size_t mask = 0; mask < totalSubsets; mask++)
	{
		/*
		 *	Count the number of set bits in this subset.
		 */
		size_t  bitCount    = 0;
		size_t  tmp         = mask;

		while (tmp != 0)
		{
			bitCount    += tmp & 1;
			tmp         >>= 1;
		}

		if (bitCount < kRequired)
		{
			continue;
		}

		/*
		 *	Compute the probability of this exact subset:
		 *	  product of p_i for each set bit,
		 *	  product of (1 - p_i) for each unset bit.
		 */
		double subsetProbability = 1.0;

		for (size_t ii = 0; ii < numberOfChildren; ii++)
		{
			if (mask & ((size_t) 1 << ii))
			{
				subsetProbability *= childProbabilities[ii];
			}
			else
			{
				subsetProbability *= (1.0 - childProbabilities[ii]);
			}
		}

		result += subsetProbability;
	}

	return result;
}

double
praEvaluateFaultTree(
	PraFaultTreeNode *  node,
	double *            basicEventProbabilities,
	size_t              numberOfBasicEvents)
{
	if (node->isBasicEvent)
	{
		return basicEventProbabilities[node->basicEventIndex];
	}

	/*
	 *	Recursively evaluate all children.
	 */
	double childProbabilities[kPraFaultTreeConstantMaxChildren];

	for (size_t ii = 0; ii < node->numberOfChildren; ii++)
	{
		childProbabilities[ii] = praEvaluateFaultTree(
			node->children[ii],
			basicEventProbabilities,
			numberOfBasicEvents
		);
	}

	switch (node->gateType)
	{
		case kPraGateTypeAnd:
		{
			/*
			 *	AND gate: P = product(P_i)
			 *	Reference: NUREG-0492, Section 5.2.1.
			 */
			double product = 1.0;

			for (size_t ii = 0; ii < node->numberOfChildren; ii++)
			{
				product *= childProbabilities[ii];
			}

			return product;
		}

		case kPraGateTypeOr:
		{
			/*
			 *	OR gate: P = 1 - product(1 - P_i)
			 *	This is exact for independent events.
			 *	Reference: NUREG-0492, Section 5.2.2.
			 */
			double survivalProduct = 1.0;

			for (size_t ii = 0; ii < node->numberOfChildren; ii++)
			{
				survivalProduct *= (1.0 - childProbabilities[ii]);
			}

			return 1.0 - survivalProduct;
		}

		case kPraGateTypeKOfN:
		{
			return praEvaluateKOfNGate(
				childProbabilities,
				node->numberOfChildren,
				node->kOfN
			);
		}

		default:
		{
			return 0.0;
		}
	}
}
