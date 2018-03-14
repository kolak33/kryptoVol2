#pragma once
#include <vector>

typedef struct LCGparams
{
	LL M; // multiplier
	LL C; // increment
	LL N; // modulus
	LL State; // current state
}LCGParams;

class LCGBreaker
{
public:
	LCGBreaker();

	LL CompIncrement(std::vector<LL> &states, LL lModulus, LL lMultiplier);
	LL CompMultiplier(std::vector<LL> &states, LL lModulus);
	LL CompModulus(std::vector<LL> &states);
	
	void FindLCGParameters(std::vector<LL> &states);
	LL PredictNext();
private:
	LL GCDExt(LL a, LL b, LL &x, LL &y);
	LL ModInv(LL b, LL n);
	LL GCD(LL a, LL b);
	LL PMod(LL i, LL n); // positive i % n
	
	bool m_bPredictedParams;
	LCGParams m_lcgParams;
};

