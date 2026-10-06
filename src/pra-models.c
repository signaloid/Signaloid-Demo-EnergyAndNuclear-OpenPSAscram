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
 *	pra-models.c — Hard-coded fault tree models for PRA demonstration.
 *
 *	Each model uses statically-allocated nodes and arrays. The tree
 *	structure is built by assigning child pointers to pre-allocated nodes.
 *
 *	References:
 *	  - Vesely et al. (1981), NUREG-0492.
 *	  - SCRAM input corpus: input/SmallTree/SmallTree.xml.
 */

#include <string.h>
#include <stdio.h>
#include "pra-models.h"

/*
 *	==========================================================================
 *	Model 0: SmallTree — top = (e1 AND e2) OR (e3 AND e4)
 *
 *	Derived from scram-upstream/input/SmallTree/SmallTree.xml.
 *	4 basic events, 3 gates.
 *	All lognormal: e1(mean=2e-5,EF=3), e2(mean=1e-5,EF=3),
 *	               e3(mean=2e-5,EF=3), e4(mean=1e-5,EF=3).
 *	lambda4 references lambda2 in the XML, so e4 = e2 parameters.
 *	==========================================================================
 */
static PraFaultTreeNode smallTreeLeafE1 = { .isBasicEvent = true, .basicEventIndex = 0 };
static PraFaultTreeNode smallTreeLeafE2 = { .isBasicEvent = true, .basicEventIndex = 1 };
static PraFaultTreeNode smallTreeLeafE3 = { .isBasicEvent = true, .basicEventIndex = 2 };
static PraFaultTreeNode smallTreeLeafE4 = { .isBasicEvent = true, .basicEventIndex = 3 };

static PraFaultTreeNode smallTreeGateG1 = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeAnd,
	.numberOfChildren   = 2,
	.children           = { &smallTreeLeafE1, &smallTreeLeafE2 },
};

static PraFaultTreeNode smallTreeGateG2 = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeAnd,
	.numberOfChildren   = 2,
	.children           = { &smallTreeLeafE3, &smallTreeLeafE4 },
};

static PraFaultTreeNode smallTreeTop = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeOr,
	.numberOfChildren   = 2,
	.children           = { &smallTreeGateG1, &smallTreeGateG2 },
};

void
praBuildSmallTreeModel(PraFaultTreeModel * model)
{
	snprintf(model->modelName, kPraFaultTreeConstantMaxNameLength, "SmallTree");

	model->topGate              = &smallTreeTop;
	model->numberOfBasicEvents  = 4;

	snprintf(model->basicEventNames[0], kPraFaultTreeConstantMaxNameLength, "e1");
	snprintf(model->basicEventNames[1], kPraFaultTreeConstantMaxNameLength, "e2");
	snprintf(model->basicEventNames[2], kPraFaultTreeConstantMaxNameLength, "e3");
	snprintf(model->basicEventNames[3], kPraFaultTreeConstantMaxNameLength, "e4");

	model->basicEvents[0] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 2.0e-5, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[1] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-5, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[2] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 2.0e-5, .param2 = 3.0, .param3 = 0.95
	};
	/*
	 *	lambda4 references lambda2 in the XML.
	 */
	model->basicEvents[3] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-5, .param2 = 3.0, .param3 = 0.95
	};

	return;
}

/*
 *	==========================================================================
 *	Model 1: TwoTrain — top = Train1 AND Train2
 *	  Train1 = Valve1 OR Pump1
 *	  Train2 = Valve2 OR Pump2
 *
 *	Classic PRA redundancy pattern from NUREG-0492.
 *	4 basic events with varying lognormal parameters.
 *	==========================================================================
 */
static PraFaultTreeNode twoTrainLeafValve1  = { .isBasicEvent = true, .basicEventIndex = 0 };
static PraFaultTreeNode twoTrainLeafPump1   = { .isBasicEvent = true, .basicEventIndex = 1 };
static PraFaultTreeNode twoTrainLeafValve2  = { .isBasicEvent = true, .basicEventIndex = 2 };
static PraFaultTreeNode twoTrainLeafPump2   = { .isBasicEvent = true, .basicEventIndex = 3 };

static PraFaultTreeNode twoTrainGateTrain1 = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeOr,
	.numberOfChildren   = 2,
	.children           = { &twoTrainLeafValve1, &twoTrainLeafPump1 },
};

static PraFaultTreeNode twoTrainGateTrain2 = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeOr,
	.numberOfChildren   = 2,
	.children           = { &twoTrainLeafValve2, &twoTrainLeafPump2 },
};

static PraFaultTreeNode twoTrainTop = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeAnd,
	.numberOfChildren   = 2,
	.children           = { &twoTrainGateTrain1, &twoTrainGateTrain2 },
};

void
praBuildTwoTrainModel(PraFaultTreeModel * model)
{
	snprintf(model->modelName, kPraFaultTreeConstantMaxNameLength, "TwoTrain");

	model->topGate              = &twoTrainTop;
	model->numberOfBasicEvents  = 4;

	snprintf(model->basicEventNames[0], kPraFaultTreeConstantMaxNameLength, "Valve1");
	snprintf(model->basicEventNames[1], kPraFaultTreeConstantMaxNameLength, "Pump1");
	snprintf(model->basicEventNames[2], kPraFaultTreeConstantMaxNameLength, "Valve2");
	snprintf(model->basicEventNames[3], kPraFaultTreeConstantMaxNameLength, "Pump2");

	model->basicEvents[0] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-3, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[1] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-4, .param2 = 10.0, .param3 = 0.95
	};
	model->basicEvents[2] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-3, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[3] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-4, .param2 = 10.0, .param3 = 0.95
	};

	return;
}

/*
 *	==========================================================================
 *	Model 2: HIPPS — High Integrity Pressure Protection System
 *	  top = Sensor OR LogicSolver OR FinalElement
 *	  Sensor = 2-of-3(S1, S2, S3)
 *
 *	Demonstrates K-of-N voting gates.
 *	6 basic events, 3 gates.
 *	==========================================================================
 */
static PraFaultTreeNode hippsLeafS1     = { .isBasicEvent = true, .basicEventIndex = 0 };
static PraFaultTreeNode hippsLeafS2     = { .isBasicEvent = true, .basicEventIndex = 1 };
static PraFaultTreeNode hippsLeafS3     = { .isBasicEvent = true, .basicEventIndex = 2 };
static PraFaultTreeNode hippsLeafLogic  = { .isBasicEvent = true, .basicEventIndex = 3 };
static PraFaultTreeNode hippsLeafValve1 = { .isBasicEvent = true, .basicEventIndex = 4 };
static PraFaultTreeNode hippsLeafValve2 = { .isBasicEvent = true, .basicEventIndex = 5 };

static PraFaultTreeNode hippsGateSensor = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeKOfN,
	.kOfN               = 2,
	.numberOfChildren   = 3,
	.children           = { &hippsLeafS1,  &hippsLeafS2,&hippsLeafS3 },
};

static PraFaultTreeNode hippsGateFinalElement = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeAnd,
	.numberOfChildren   = 2,
	.children           = { &hippsLeafValve1, &hippsLeafValve2 },
};

static PraFaultTreeNode hippsTop = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeOr,
	.numberOfChildren   = 3,
	.children           = { &hippsGateSensor, &hippsLeafLogic, &hippsGateFinalElement },
};

void
praBuildHippsModel(PraFaultTreeModel * model)
{
	snprintf(model->modelName, kPraFaultTreeConstantMaxNameLength, "HIPPS");

	model->topGate              = &hippsTop;
	model->numberOfBasicEvents  = 6;

	snprintf(model->basicEventNames[0], kPraFaultTreeConstantMaxNameLength, "Sensor1");
	snprintf(model->basicEventNames[1], kPraFaultTreeConstantMaxNameLength, "Sensor2");
	snprintf(model->basicEventNames[2], kPraFaultTreeConstantMaxNameLength, "Sensor3");
	snprintf(model->basicEventNames[3], kPraFaultTreeConstantMaxNameLength, "LogicSolver");
	snprintf(model->basicEventNames[4], kPraFaultTreeConstantMaxNameLength, "FinalValve1");
	snprintf(model->basicEventNames[5], kPraFaultTreeConstantMaxNameLength, "FinalValve2");

	model->basicEvents[0] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 5.0e-3, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[1] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 5.0e-3, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[2] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 5.0e-3, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[3] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-4, .param2 = 5.0, .param3 = 0.95
	};
	model->basicEvents[4] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-3, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[5] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-3, .param2 = 3.0, .param3 = 0.95
	};

	return;
}

/*
 *	==========================================================================
 *	Model 3: SeriesSystem — top = e1 OR e2 OR e3 OR e4 OR e5
 *
 *	Simple series system for well-characterized MC-vs-UxHw comparison.
 *	5 basic events, 1 gate.
 *	==========================================================================
 */
static PraFaultTreeNode seriesLeafE1    = { .isBasicEvent = true, .basicEventIndex = 0 };
static PraFaultTreeNode seriesLeafE2    = { .isBasicEvent = true, .basicEventIndex = 1 };
static PraFaultTreeNode seriesLeafE3    = { .isBasicEvent = true, .basicEventIndex = 2 };
static PraFaultTreeNode seriesLeafE4    = { .isBasicEvent = true, .basicEventIndex = 3 };
static PraFaultTreeNode seriesLeafE5    = { .isBasicEvent = true, .basicEventIndex = 4 };

static PraFaultTreeNode seriesTop = {
	.isBasicEvent       = false,
	.gateType           = kPraGateTypeOr,
	.numberOfChildren   = 5,
	.children           = { &seriesLeafE1, &seriesLeafE2, &seriesLeafE3, &seriesLeafE4, &seriesLeafE5 },
};

void
praBuildSeriesSystemModel(PraFaultTreeModel * model)
{
	snprintf(model->modelName, kPraFaultTreeConstantMaxNameLength, "SeriesSystem");

	model->topGate              = &seriesTop;
	model->numberOfBasicEvents  = 5;

	snprintf(model->basicEventNames[0], kPraFaultTreeConstantMaxNameLength, "e1");
	snprintf(model->basicEventNames[1], kPraFaultTreeConstantMaxNameLength, "e2");
	snprintf(model->basicEventNames[2], kPraFaultTreeConstantMaxNameLength, "e3");
	snprintf(model->basicEventNames[3], kPraFaultTreeConstantMaxNameLength, "e4");
	snprintf(model->basicEventNames[4], kPraFaultTreeConstantMaxNameLength, "e5");

	model->basicEvents[0] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-3, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[1] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 2.0e-3, .param2 = 5.0, .param3 = 0.95
	};
	model->basicEvents[2] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 5.0e-4, .param2 = 3.0, .param3 = 0.95
	};
	model->basicEvents[3] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 1.0e-3, .param2 = 10.0, .param3 = 0.95
	};
	model->basicEvents[4] = (PraBasicEventDistribution){
		.distributionType   = kPraDistributionTypeLognormal,
		.param1             = 3.0e-3, .param2 = 3.0, .param3 = 0.95
	};

	return;
}
