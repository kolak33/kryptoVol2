#include "stdafx.h"
#include "LCG.h"


LCG::LCG(LL lSeed, LL lM, LL lC, LL lN)
	: m_lState(lSeed),
	m_M(lM),
	m_C(lC),
	m_N(lN)
{
}

void LCG::SetSeed(LL lSeed)
{
	m_lState = lSeed;
}

LL LCG::GenNext()
{
	ATLASSERT(m_N != 0);
	m_lState = PMod((m_lState * m_M + m_C), m_N);
	return m_lState;
}

LL LCG::PMod(LL i, LL n)
{
	return (i % n + n) % n;
}