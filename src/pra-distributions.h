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
 *	pra-distributions.h — Distribution parameter conversion for PRA basic events.
 *
 *	Ports the lognormal error-factor-to-(mu,sigma) conversion from the
 *	SCRAM Signaloid integration (signaloid-uxhwDistributions.cc) to pure C.
 *
 *	References:
 *	  - NUREG/CR-6823 (2003), "Handbook of Parameter Estimation for PRA",
 *	    Section 6.2 (lognormal distribution).
 *	  - Open-PSA MEF v2.0, Chapter 5 (Stochastic Layer).
 *	  - Signaloid UxHw API: https://docs.signaloid.io/docs/uxhw-api/
 */

#pragma once

#include "pra-faultTree.h"

/**
 *	@brief	Convert lognormal MEF parameters (mean, error factor, confidence
 *		level) to standard lognormal (mu, sigma) parameters.
 *
 *	The error factor EF at confidence level p is defined such that:
 *	  P(X <= mean * EF) = p
 *
 *	From which (NUREG/CR-6823, Section 6.2):
 *	  sigma = ln(EF) / z_p
 *	  mu    = ln(mean) - sigma^2 / 2
 *
 *	where z_p = Phi^{-1}(p) is the p-quantile of the standard normal.
 *
 *	@param	mean		: Mean of the lognormal distribution.
 *	@param	errorFactor	: Error factor (EF).
 *	@param	confidenceLevel	: Confidence level (e.g., 0.95).
 *	@param	outMu		: Output: mu parameter of underlying normal.
 *	@param	outSigma	: Output: sigma parameter of underlying normal.
 */
void
praLognormalMefToMuSigma(
	double      mean,
	double      errorFactor,
	double      confidenceLevel,
	double *    outMu,
	double *    outSigma);

/**
 *	@brief	Create a distributional double for a basic event using the
 *		UxHw API.
 *
 *	Dispatches on distribution type to the appropriate UxHw function:
 *	  - Point:     returns param1 directly
 *	  - Lognormal: converts EF to (mu, sigma), calls UxHwDoubleLognormalDist
 *	  - Uniform:   calls UxHwDoubleUniformDist(min, max)
 *	  - Normal:    calls UxHwDoubleGaussDist(mean, stddev)
 *
 *	On SCCE, these create distributional doubles. On native platforms,
 *	the GSL-based compat stubs sample from the distributions.
 *
 *	@param	distribution	: Pointer to basic event distribution parameters.
 *	@return	double		: Distributional (or sampled) basic event probability.
 */
double
praCreateDistributionalBasicEvent(
	PraBasicEventDistribution * distribution);
