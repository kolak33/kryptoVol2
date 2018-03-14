#pragma once

class LCG
{
public:
	LCG(LL lSeed = 1, LL lM = 0, LL lC = 0, LL lN = 0);

	void SetSeed(LL lSeed);
	LL GenNext();

private:
	LL m_M; // multiplier
	LL m_C; // increment
	LL m_N; // modulus
	LL m_lState; // current state

	LL PMod(LL i, LL n); // positive i % n
};

