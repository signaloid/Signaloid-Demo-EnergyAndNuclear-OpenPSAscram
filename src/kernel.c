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
 *	kernel.c — PRA fault tree evaluation kernel.
 *
 *	For each selected output, builds a fault tree model, creates
 *	distributional basic event probabilities, and evaluates the tree
 *	to produce the top-event probability.
 *
 *	References:
 *	  - Vesely et al. (1981), NUREG-0492.
 *	  - NUREG/CR-6823 (2003) for distribution parameters.
 */

#include "kernel.h"
#include "pra-faultTree.h"
#include "pra-distributions.h"
#include "pra-models.h"

/**
 *	@brief	Evaluate a single PRA model: build tree, create distributional
 *		basic events, evaluate, and return top-event probability.
 *
 *	@param	model	: Pointer to the fault tree model to evaluate.
 *	@return	double	: Top-event probability (distributional on SCCE).
 */
static double
evaluateModel(PraFaultTreeModel * model)
{
	/*
	 *	PSM: File issue to implement: This should be alloca()'d
	 *	based on model->numberOfBasicEvents (could also malloc it
	 *	and first save the return of praEvaluateFaultTree() and
	 *	then free it before turning)
	 */
	double basicEventProbabilities[kPraFaultTreeConstantMaxBasicEvents];

	/*
	 *	PSM: File issue to implement: The concept of the basic event
	 *	probabilities is subtle, and requires detailed comment annotation
	 *	in the code and also the README.md: When tools like SCRAM in
	 *	OpenPSA let you do Monte Carlo uncertainty analysis, those
	 *	uncertainties are epistemic uncertainties about the failure
	 *	probabilities. That is, based on empirical observations of
	 *	failures of one component captured in the tree, different
	 *	observers (e.g., different factories, different supply chains,
	 *	etc.) might have observed different failure rates (i.e., different
	 *	empirical failure probabilities). Across all the observers, there
	 *	is a distribution of this failure rate/probability, with the mode
	 *	being the most commonly observed failure probability, etc.
	 *
	 *	As part of the implementation of this issue, we should implement
	 *	sanity checks to make sure that the distributions of the failure
	 *	probabilities have support that makes sense for probabilities.
	 */
	for (size_t ii = 0; ii < model->numberOfBasicEvents; ii++)
	{
		basicEventProbabilities[ii] = praCreateDistributionalBasicEvent(
			&model->basicEvents[ii]
		);
	}

	return praEvaluateFaultTree(model->topGate, basicEventProbabilities, model->numberOfBasicEvents);
}

void
calculateOutput(
	CommandLineArguments *  arguments,
	double *                outputVariables)
{
	bool calculateAllOutputs = (arguments->common.outputSelect == kOutputDistributionIndexMax);

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputDistributionIndexSmallTree))
	{
		PraFaultTreeModel model;

		praBuildSmallTreeModel(&model);
		outputVariables[kOutputDistributionIndexSmallTree] = evaluateModel(&model);
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputDistributionIndexTwoTrain))
	{
		PraFaultTreeModel model;

		praBuildTwoTrainModel(&model);
		outputVariables[kOutputDistributionIndexTwoTrain] = evaluateModel(&model);
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputDistributionIndexHipps))
	{
		PraFaultTreeModel model;

		praBuildHippsModel(&model);
		outputVariables[kOutputDistributionIndexHipps] = evaluateModel(&model);
	}

	if (calculateAllOutputs || (arguments->common.outputSelect == kOutputDistributionIndexSeriesSystem))
	{
		PraFaultTreeModel model;

		praBuildSeriesSystemModel(&model);
		outputVariables[kOutputDistributionIndexSeriesSystem] = evaluateModel(&model);
	}

	return;
}
