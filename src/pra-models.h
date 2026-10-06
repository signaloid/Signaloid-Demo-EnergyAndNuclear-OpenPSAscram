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
 *	pra-models.h — Hard-coded fault tree models for PRA demonstration.
 *
 *	Each model is built from statically-allocated data structures (no heap
 *	allocation) and represents a representative PRA fault tree topology.
 *
 *	References:
 *	  - Vesely et al. (1981), NUREG-0492, "Fault Tree Handbook".
 *	  - SCRAM input corpus: input/SmallTree/SmallTree.xml.
 */

#pragma once

#include "pra-faultTree.h"

/**
 *	@brief	Build the SmallTree model: top = (e1 AND e2) OR (e3 AND e4).
 *
 *	Derived from scram-upstream/input/SmallTree/SmallTree.xml.
 *	4 basic events with lognormal distributions (mean ~2e-5, EF=3, level=0.95).
 *	3 gates (top=OR, g1=AND, g2=AND).
 *
 *	@param	model	: Pointer to model struct to populate.
 */
void
praBuildSmallTreeModel(PraFaultTreeModel * model);

/**
 *	@brief	Build the TwoTrain redundancy model: top = Train1 AND Train2,
 *		where each Train = Valve OR Pump.
 *
 *	Classic PRA redundancy pattern from NUREG-0492.
 *	4 basic events with varying lognormal parameters.
 *	3 gates (top=AND, train1=OR, train2=OR).
 *
 *	@param	model	: Pointer to model struct to populate.
 */
void
praBuildTwoTrainModel(PraFaultTreeModel * model);

/**
 *	@brief	Build the HIPPS safety system model with a 2-of-3 voting gate.
 *
 *	Sensor subsystem uses a K-of-N (2-of-3) voting gate.
 *	6 basic events, 3 gates.
 *
 *	@param	model	: Pointer to model struct to populate.
 */
void
praBuildHippsModel(PraFaultTreeModel * model);

/**
 *	@brief	Build a series system: top = e1 OR e2 OR e3 OR e4 OR e5.
 *
 *	Well-characterized output for MC-vs-UxHw accuracy comparison.
 *	5 basic events, 1 gate.
 *
 *	@param	model	: Pointer to model struct to populate.
 */
void
praBuildSeriesSystemModel(PraFaultTreeModel * model);
