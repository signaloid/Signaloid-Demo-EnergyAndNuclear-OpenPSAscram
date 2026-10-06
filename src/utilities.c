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

#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <uxhw.h>
#include <assert.h>
#include <float.h>
#include "utilities.h"
#include "common.h"

void
printUsage(void)
{
	fprintf(stderr, "PRA Fault Tree Uncertainty Propagation Demo\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Evaluates fault tree models from nuclear probabilistic risk assessment\n");
	fprintf(stderr, "with uncertain basic event probabilities (lognormal distributions).\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Available models (-S):\n");
	fprintf(stderr, "  0: SmallTree     — (e1*e2) + (e3*e4), 4 events, 3 gates\n");
	fprintf(stderr, "  1: TwoTrain      — Redundant train system, 4 events, 3 gates\n");
	fprintf(stderr, "  2: HIPPS         — Safety system with 2-of-3 voting, 6 events, 3 gates\n");
	fprintf(stderr, "  3: SeriesSystem  — 5-event series, 5 events, 1 gate\n");
	fprintf(stderr, "  4: All outputs\n");
	fprintf(stderr, "\n");
	printCommonUsage();
	fprintf(stderr, "\n");

	return;
}

/**
 *	@brief	Set the default values for the command line arguments.
 *
 *	@param	arguments	: command line arguments pointer.
 *	@return			: `kCommonConstantReturnTypeSuccess` if successful, else `kCommonConstantReturnTypeError`.
 */
static CommonConstantReturnType
setDefaultCommandLineArguments(CommandLineArguments * arguments)
{
	if (arguments == NULL)
	{
		fprintf(stderr, "Error: The provided pointer to arguments is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Older GCC versions have a bug which gives a spurious warning for the C universal zero
	 *	initializer `{0}`. Any workaround makes the code less portable or prevents the common code
	 *	from adding new fields to the `CommonCommandLineArguments` struct. Therefore, we surpress
	 *	this warning.
	 *
	 *	See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119.
	 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
	*arguments = (CommandLineArguments) {
		.common = (CommonCommandLineArguments) { 0 },
	};
#pragma GCC diagnostic pop

	return kCommonConstantReturnTypeSuccess;
}

CommonConstantReturnType
getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments)
{
	if (arguments == NULL)
	{
		fprintf(stderr, "Error: The provided pointer to arguments is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	if (setDefaultCommandLineArguments(arguments) != kCommonConstantReturnTypeSuccess)
	{
		return kCommonConstantReturnTypeError;
	}

	DemoOption options[] = {
		{ 0 },
	};

	if (parseArgs(argc, argv, &arguments->common, options) != kCommonConstantReturnTypeSuccess)
	{
		fprintf(stderr, "Error: Parsing command line arguments failed.\n");
		printUsage();

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isHelpEnabled)
	{
		printUsage();

		exit(EXIT_SUCCESS);
	}

	/*
	 *	If no output is selected, set `outputSelect` to `kOutputDistributionIndexMax`.
	 *	This triggers the demo to compute all outputs.
	 */
	if (!arguments->common.isOutputSelected)
	{
		arguments->common.outputSelect = kOutputDistributionIndexMax;
	}

	/*
	 *	When `outputSelect` is set to `kOutputDistributionIndexMax`, we cannot be
	 *	in benchmarking mode or Monte Carlo mode.
	 */
	if (arguments->common.outputSelect == kOutputDistributionIndexMax)
	{
		if ((arguments->common.isBenchmarkingMode) || (arguments->common.isMonteCarloMode))
		{
			fprintf(stderr, "Error: Please select a single output when in benchmarking mode or Monte Carlo mode.\n");

			return kCommonConstantReturnTypeError;
		}
	}
	/*
	 *	Selected output can never be greater than `kOutputDistributionIndexMax`.
	 */
	else if (arguments->common.outputSelect > kOutputDistributionIndexMax)
	{
		fprintf(stderr, "Error: Wrong output selection.\n");

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isVerbose)
	{
		fprintf(stderr, "Warning: Verbose mode not supported. Continuing in non-verbose mode.\n");
	}

	return kCommonConstantReturnTypeSuccess;
}
