#include "stdafx.h"
#include "LCGBreaker.h"

#include <iostream>

LCGBreaker::LCGBreaker()
	: m_bPredictedParams(false)
{
}

void LCGBreaker::FindLCGParameters(std::vector<LL> &states)
{
	LL lModulus = CompModulus(states);
	LL lMultiplier = CompMultiplier(states, lModulus);
	LL lIncrement = CompIncrement(states, lModulus, lMultiplier);

	m_lcgParams.N = lModulus;
	m_lcgParams.M = lMultiplier;
	m_lcgParams.C = lIncrement;
	m_lcgParams.State = states.back();
	m_bPredictedParams = true;
}

LL LCGBreaker::PredictNext()
{
	if (!m_bPredictedParams)
		return -1;

	ATLASSERT(m_lcgParams.N != 0);
	m_lcgParams.State = PMod((m_lcgParams.State * m_lcgParams.M + m_lcgParams.C), m_lcgParams.N);
	return m_lcgParams.State;
}

LL LCGBreaker::CompIncrement(std::vector<LL> &states, LL lModulus, LL lMultiplier)
{
	// s1 = s0*m + c	(mod n)
	// c = s1 - s0*m	(mod n)
	ATLASSERT(states.size() >= 2);
	return PMod((states[1] - states[0] * lMultiplier), lModulus);
}

LL LCGBreaker::CompMultiplier(std::vector<LL> &states, LL lModulus)
{
	// s2 - s1 = m * (s1 - s0)	 (mod n)
	// m = (s2 - s1) / (s1 - s0)	(mod n)
	ATLASSERT(states.size() >= 3);
	LL lMultiplier = PMod((states[2] - states[1]) * ModInv(states[1] - states[0], lModulus), lModulus);
	return lMultiplier;
}

LL LCGBreaker::CompModulus(std::vector<LL> &states)
{
	//t0 = s1 - s0
	//t1 = s2 - s1 = (s1*m + c) - (s0*m + c) = m * (s1 - s0) = m * t0(mod n)
	//t2 = s3 - s2 = (s2*m + c) - (s1*m + c) = m * (s2 - s1) = m * t1(mod n)
	//t3 = s4 - s3 = (s3*m + c) - (s2*m + c) = m * (s3 - s2) = m * t2(mod n)
	//t2*t0 - t1*t1 = (m*m*t0 * t0) - (m*t0 * m*t0) = 0 (mod n)
	ATLASSERT(states.size() >= 6);
	std::vector<LL> diffs;
	for (size_t i = 0; i < states.size() - 1; ++i)
		diffs.push_back(states[i + 1] - states[i]);

	std::vector<LL> congruentZeroes;
	for (size_t i = 0; i < diffs.size() - 2; ++i)
		congruentZeroes.push_back(diffs[i + 2] * diffs[i] - diffs[i + 1] * diffs[i + 1]);

	LL lGCD = congruentZeroes[0];
	for (size_t i = 1; i < congruentZeroes.size(); ++i)
		lGCD = GCD(lGCD, congruentZeroes[i]);

	LL lModulus = std::abs(lGCD);
	return lModulus;
}

LL LCGBreaker::GCD(LL a, LL b)
{
	if (a == 0) return b;
	return GCD(b%a, a);
}

LL LCGBreaker::ModInv(LL b, LL n)
{
	LL x, y;
	LL res = -1;
	LL g = GCDExt(b, n, x, y);
	if (g != 1)
		std::cout << "Inverse doesn't exist\n";
	else
		res = PMod(x, n);

	return res;
}

LL LCGBreaker::GCDExt(LL a, LL b, LL &x, LL &y)
{
	if (a == 0)
	{
		x = 0;
		y = 1;
		return b;
	}

	LL x1, y1; // To store results of recursive call
	LL gcd = GCDExt(b%a, a, x1, y1);

	// Update x and y using results of recursive call
	x = y1 - (b / a) * x1;
	y = x1;

	return gcd;
}

LL LCGBreaker::PMod(LL i, LL n)
{
	return (i % n + n) % n;
}