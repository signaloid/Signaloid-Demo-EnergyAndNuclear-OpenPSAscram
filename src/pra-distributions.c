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
 *	pra-distributions.c — Distribution parameter conversion and creation
 *	for PRA basic events.
 *
 *	Ported from the SCRAM Signaloid integration
 *	(signaloid-uxhwDistributions.cc) to pure C, removing the Boost
 *	dependency by using a rational approximation for the inverse normal CDF.
 *
 *	References:
 *	  - NUREG/CR-6823 (2003), Section 6.2.
 *	  - Acklam, P. J. (2003), "An algorithm for computing the inverse
 *	    normal cumulative distribution function".
 *	  - Abramowitz & Stegun (1964), Section 26.2.
 */

#include <math.h>
#include <uxhw.h>
#include "pra-distributions.h"

/*
 *	Hardcoded inverse normal CDF value for the 95% confidence level,
 *	which covers nearly all PRA lognormal parameterizations.
 *
 *	z_{0.95} = Phi^{-1}(0.95) = 1.6448536269514729...
 *
 *	Reference: Abramowitz & Stegun (1964), Table 26.1.
 */
static const double kPraConstantInverseNormalCdf095 = 1.6448536269514729;

/**
 *	@brief	Compute the inverse normal CDF (probit function) using
 *		Peter Acklam's rational approximation.
 *
 *	This approximation has a relative error of less than 1.15e-9
 *	for all p in (0, 1).
 *
 *	Reference: Acklam, P. J. (2003), "An algorithm for computing the
 *	inverse normal cumulative distribution function".
 *
 *	@param	p	: Probability in (0, 1).
 *	@return	double	: z such that Phi(z) = p.
 */
static double
praInverseNormalCdf(double p)
{
	/*
	 *	Coefficients for Acklam's rational approximation.
	 */
	static const double aa[] = {
		-3.969683028665376e+01,
		2.209460984245205e+02,
		-2.759285104469687e+02,
		1.383577518672690e+02,
		-3.066479806614716e+01,
		2.506628277459239e+00
	};

	static const double bb[] = {
		-5.447609879822406e+01,
		1.615858368580409e+02,
		-1.556989798598866e+02,
		6.680131188771972e+01,
		-1.328068155288572e+01
	};

	static const double cc[] = {
		-7.784894002430293e-03,
		-3.223964580411365e-01,
		-2.400758277161838e+00,
		-2.549732539343734e+00,
		4.374664141464968e+00,
		2.938163982698783e+00
	};

	static const double dd[] = {
		7.784695709041462e-03,
		3.224671290700398e-01,
		2.445134137142996e+00,
		3.754408661907416e+00
	};

	static const double pLow    = 0.02425;
	static const double pHigh   = 1.0 - 0.02425;

	double  qq;
	double  rr;

	if (p < pLow)
	{
		/*
		 *	Lower tail approximation.
		 */
		qq = sqrt(-2.0 * log(p));

		return (((((cc[0] * qq + cc[1]) * qq + cc[2]) * qq + cc[3]) * qq + cc[4]) * qq + cc[5]) /
		       ((((dd[0] * qq + dd[1]) * qq + dd[2]) * qq + dd[3]) * qq + 1.0);
	}
	else if (p <= pHigh)
	{
		/*
		 *	Central approximation.
		 */
		qq  = p - 0.5;
		rr  = qq * qq;

		return (((((aa[0] * rr + aa[1]) * rr + aa[2]) * rr + aa[3]) * rr + aa[4]) * rr + aa[5]) * qq /
		       (((((bb[0] * rr + bb[1]) * rr + bb[2]) * rr + bb[3]) * rr + bb[4]) * rr + 1.0);
	}
	else
	{
		/*
		 *	Upper tail approximation.
		 */
		qq = sqrt(-2.0 * log(1.0 - p));

		return -(((((cc[0] * qq + cc[1]) * qq + cc[2]) * qq + cc[3]) * qq + cc[4]) * qq + cc[5]) /
		       ((((dd[0] * qq + dd[1]) * qq + dd[2]) * qq + dd[3]) * qq + 1.0);
	}
}

/*
 *	PSM: File issue to implement: MEF is the format used by SCRAM
 *	and OpenPSA. Add comments in code and in README.md to document
 *	a bit more about the way MEF generally represents distributions
 *	and why.
 */
void
praLognormalMefToMuSigma(
	double      mean,
	double      errorFactor,
	double      confidenceLevel,
	double *    outMu,
	double *    outSigma)
{
	double zLevel;

	/*
	 *	Use the hardcoded value for the common 95% confidence case;
	 *	fall back to the rational approximation for other levels.
	 *
	 *	z_p = Phi^{-1}(confidenceLevel)
	 */
	if (confidenceLevel == 0.95)
	{
		zLevel = kPraConstantInverseNormalCdf095;
	}
	else
	{
		zLevel = praInverseNormalCdf(confidenceLevel);
	}

	/*
	 *	sigma = ln(EF) / z_p
	 *	mu    = ln(mean) - sigma^2 / 2
	 *
	 *	Reference: NUREG/CR-6823 (2003), Section 6.2, Equation 6.2-3.
	 */
	*outSigma   = log(errorFactor) / zLevel;
	*outMu      = log(mean) - (*outSigma * *outSigma) / 2.0;

	return;
}

double
praCreateDistributionalBasicEvent(
	PraBasicEventDistribution * distribution)
{
	switch (distribution->distributionType)
	{
		case kPraDistributionTypePoint:
		{
			return distribution->param1;
		}

		case kPraDistributionTypeLognormal:
		{
			double  mu;
			double  sigma;

			praLognormalMefToMuSigma(
				distribution->param1,
				distribution->param2,
				distribution->param3,
				&mu,
				&sigma
			);

			return UxHwDoubleLognormalDist(mu, sigma);
		}

		case kPraDistributionTypeUniform:
		{
			return UxHwDoubleUniformDist(
				distribution->param1,
				distribution->param2
			);
		}

		case kPraDistributionTypeNormal:
		{
			return UxHwDoubleGaussDist(
				distribution->param1,
				distribution->param2
			);
		}

		default:
		{
			return 0.0;
		}
	}
}
