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

#pragma once

#include <stdlib.h>
#include <stdbool.h>

typedef enum
{
	kOutputDistributionIndexSmallTree = 0,
	kOutputDistributionIndexTwoTrain,
	kOutputDistributionIndexHipps,
	kOutputDistributionIndexSeriesSystem,
	kOutputDistributionIndexMax,
} OutputDistributionIndex;

/**
 *	@brief	UxHw-mode calculation kernel. Evaluates the selected PRA fault
 *		tree model(s) once. The distributional basic event probabilities
 *		propagate through the tree via the UxHw API.
 *
 *	@param	outputSelect	: Index of the output to calculate, or
 *				  `kOutputDistributionIndexMax` for all outputs.
 *	@param	outputVariables	: The output variables.
 */
void
calculateOutputUxHw(
	size_t      outputSelect,
	double *    outputVariables);

/**
 *	@brief	Monte Carlo calculation kernel. Evaluates the selected PRA fault
 *		tree model `numberOfMonteCarloIterations` times. Each evaluation
 *		draws a single sample for every basic event probability, and the
 *		selected output is stored in `monteCarloOutputSamples`.
 *
 *	@param	numberOfMonteCarloIterations	: Number of Monte Carlo iterations to run.
 *	@param	outputSelect		: Index of the output to calculate. A
 *					  single output, as Monte Carlo mode requires.
 *	@param	outputVariables		: The output variables.
 *	@param	monteCarloOutputSamples	: Array of `numberOfMonteCarloIterations` doubles, filled with samples.
 */
void
calculateOutputMonteCarlo(
	size_t      numberOfMonteCarloIterations,
	size_t      outputSelect,
	double *    outputVariables,
	double *    monteCarloOutputSamples);
