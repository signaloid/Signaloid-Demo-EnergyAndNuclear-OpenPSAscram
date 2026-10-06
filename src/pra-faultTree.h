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
 *	pra-faultTree.h — Fault tree data structures and gate-by-gate probability
 *	evaluation for probabilistic risk assessment (PRA).
 *
 *	References:
 *	  - Vesely et al. (1981), NUREG-0492, "Fault Tree Handbook".
 *	  - Kumamoto & Henley (1996), "Probabilistic Risk Assessment and
 *	    Management for Engineers and Scientists", 2nd ed.
 */

#pragma once

#include <stdlib.h>
#include <stdbool.h>

/**
 *	@brief	Maximum number of children per gate node.
 */
enum
{
	kPraFaultTreeConstantMaxChildren    = 8,
	kPraFaultTreeConstantMaxBasicEvents = 16,
	kPraFaultTreeConstantMaxNameLength  = 64,
};

/**
 *	@brief	Gate types for fault tree logic.
 *
 *	Reference: NUREG-0492, Section 3.3.
 */
typedef enum
{
	kPraGateTypeAnd = 0,
	kPraGateTypeOr,
	kPraGateTypeKOfN,
} PraGateType;

/**
 *	@brief	Distribution types for basic event failure probabilities.
 *
 *	Reference: NUREG/CR-6823, Chapters 5-6.
 */
typedef enum
{
	kPraDistributionTypePoint = 0,
	kPraDistributionTypeLognormal,
	kPraDistributionTypeUniform,
	kPraDistributionTypeNormal,
} PraDistributionType;

/**
 *	@brief	Distribution parameters for a basic event.
 *
 *	The interpretation of param1/param2/param3 depends on distributionType:
 *	  - Point:     param1 = value
 *	  - Lognormal: param1 = mean, param2 = error factor, param3 = confidence level
 *	  - Uniform:   param1 = min, param2 = max
 *	  - Normal:    param1 = mean, param2 = standard deviation
 */
typedef struct
{
	PraDistributionType distributionType;
	double              param1;
	double              param2;
	double              param3;
} PraBasicEventDistribution;

/**
 *	@brief	Fault tree node representing either a gate or a basic event leaf.
 *
 *	If isBasicEvent is true, basicEventIndex indexes into the model's
 *	basic event arrays. Otherwise, gateType and children define the logic.
 */
typedef struct PraFaultTreeNode
{
	bool                        isBasicEvent;
	size_t                      basicEventIndex;

	PraGateType                 gateType;
	size_t                      kOfN;
	size_t                      numberOfChildren;
	struct PraFaultTreeNode *   children[kPraFaultTreeConstantMaxChildren];
} PraFaultTreeNode;

/**
 *	@brief	Complete fault tree model with basic event distributions.
 */
typedef struct
{
	char                        modelName[kPraFaultTreeConstantMaxNameLength];
	PraFaultTreeNode *          topGate;
	PraBasicEventDistribution   basicEvents[kPraFaultTreeConstantMaxBasicEvents];
	char                        basicEventNames[kPraFaultTreeConstantMaxBasicEvents][kPraFaultTreeConstantMaxNameLength];
	size_t                      numberOfBasicEvents;
} PraFaultTreeModel;

/**
 *	@brief	Evaluate a fault tree node recursively, computing the top-event
 *		probability from basic event probabilities.
 *
 *	Gate evaluation (Vesely et al., NUREG-0492, Section 5.2):
 *	  - AND:   P = product(P_i)
 *	  - OR:    P = 1 - product(1 - P_i)  (inclusion-exclusion for independent events)
 *	  - K/N:   Combinatorial sum over C(N,K) subsets
 *
 *	All operations are pure arithmetic (no data-dependent branches on
 *	probability values) with no independence calls. The AND and OR gates
 *	use each child once, so their distributional propagation is exact on
 *	every core. The K-of-N gate reuses each child in several subset terms,
 *	so its propagation is exact only on a core that tracks correlation.
 *
 *	@param	node				: Pointer to the fault tree node to evaluate.
 *	@param	basicEventProbabilities		: Array of basic event probabilities.
 *	@param	numberOfBasicEvents		: Number of basic events in the array.
 *	@return	double				: Top-event probability.
 */
double
praEvaluateFaultTree(
	PraFaultTreeNode *  node,
	double *            basicEventProbabilities,
	size_t              numberOfBasicEvents);
