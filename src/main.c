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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <uxhw.h>
#include "utilities.h"
#include "kernel.h"


int
main(int argc, char *  argv[])
{
	CommandLineArguments    arguments = (CommandLineArguments) { 0 };
	double                  outputVariables[kOutputDistributionIndexMax];
	const char *            outputVariableNames[kOutputDistributionIndexMax] = {
		"SmallTree",
		"TwoTrain",
		"HIPPS",
		"SeriesSystem"
	};
	const char *            outputVariableDescriptions[kOutputDistributionIndexMax] = {
		"SmallTree: (e1*e2)+(e3*e4)",
		"TwoTrain: redundant train system",
		"HIPPS: safety system with 2-of-3 voting",
		"SeriesSystem: 5-event series"
	};
	const char *            applicationDescription  = "Example application for reliability estimation of nuclear power plant systems.";
	double *                monteCarloOutputSamples = NULL;
	clock_t                 start                   = 0;
	clock_t                 end                     = 0;
	double                  cpuTimeUsedInSeconds    = 0.0;

	/*
	 *	Get command line arguments.
	 */
	if (getCommandLineArguments(argc, argv, &arguments) != kCommonConstantReturnTypeSuccess)
	{
		return EXIT_FAILURE;
	}

	/*
	 *	Allocate for `monteCarloOutputSamples` if in Monte Carlo mode.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputSamples = (double *) checkedMalloc(
			arguments.common.numberOfMonteCarloIterations * sizeof(double),
			__FILE__,
			__LINE__
		);
	}

	/*
	 *	Start timing if timing is enabled.
	 */
	if (arguments.common.isTimingEnabled)
	{
		start = clock();
	}

	/*
	 *	Execute process kernel in a loop. The size of loop is 1 unless in Monte Carlo mode.
	 */
	for (size_t ii = 0; ii < arguments.common.numberOfMonteCarloIterations; ++ii)
	{
		/*
		 *	Execute process kernel.
		 */
		calculateOutput(&arguments, outputVariables);

		/*
		 *	If in Monte Carlo mode, populate `monteCarloOutputSamples`.
		 */
		if (arguments.common.isMonteCarloMode)
		{
			monteCarloOutputSamples[ii] = outputVariables[arguments.common.outputSelect];
		}
	}

	/*
	 *	Stop timing if timing is enabled.
	 */
	if (arguments.common.isTimingEnabled)
	{
		end = clock();
		cpuTimeUsedInSeconds = ((double) (end - start)) / CLOCKS_PER_SEC;
	}

	/*
	 *	Print json outputs if in JSON output mode.
	 */
	if (arguments.common.isOutputJSONMode)
	{
		printJSONFormattedOutput(
			&arguments.common,
			monteCarloOutputSamples,
			outputVariables,
			outputVariableNames,
			kOutputDistributionIndexMax,
			applicationDescription
		);
	}
	/*
	 *	Print human-consumable output if not in JSON output mode.
	 */
	else
	{
		printHumanConsumableOutput(
			&arguments.common,
			kOutputDistributionIndexMax,
			outputVariables,
			outputVariableNames,
			outputVariableDescriptions,
			monteCarloOutputSamples
		);
	}

	/*
	 *	Print timing if timing is enabled.
	 */
	if (arguments.common.isTimingEnabled)
	{
		printf("\nCPU time used: %lf seconds\n", cpuTimeUsedInSeconds);
	}

	/*
	 *	Save Monte Carlo data to "data.out" if in Monte Carlo mode.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloDoubleDataToDataDotOutFile(
			monteCarloOutputSamples,
			(uint64_t) (cpuTimeUsedInSeconds * 1000000),
			arguments.common.numberOfMonteCarloIterations
		);
	}
	/*
	 *	Save outputs to file if not in Monte Carlo mode and write to file is enabled.
	 */
	else
	{
		if (arguments.common.isWriteToFileEnabled)
		{
			if (writeOutputDoubleDistributionsToCSV(
					arguments.common.outputFilePath,
					outputVariables,
					outputVariableNames,
					kOutputDistributionIndexMax
			))
			{
				fprintf(stderr, "Error: Could not write to output CSV file \"%s\".\n", arguments.common.outputFilePath);

				return EXIT_FAILURE;
			}
		}
	}

	/*
	 *	Free allocations.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		free(monteCarloOutputSamples);
	}

	return EXIT_SUCCESS;
}
