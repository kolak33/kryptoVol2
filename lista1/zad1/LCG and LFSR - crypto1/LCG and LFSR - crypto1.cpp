// LCG and LFSR - crypto1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <atldef.h>
#include <vector>
#include <algorithm>

#include "LCG.h"
#include "LCGBreaker.h"




int main() 
{
	LL lMult = 67225731;
	LL lIncr = 73828438;
	LL lMod = 3645410033;
	LL lSeed = 123;
	LCG lcg(lSeed, lMult /*m*/, lIncr /*c*/, lMod /*n*/);
	LCGBreaker lcgBreaker;

	std::vector<LL> states = { lcg.GenNext(), lcg.GenNext(), lcg.GenNext(), lcg.GenNext(), lcg.GenNext(), lcg.GenNext(), lcg.GenNext() };
	lcgBreaker.FindLCGParameters(states);

	for (int i = 0; i < 20; ++i)
		std::cout << "prediction = " << lcgBreaker.PredictNext() << " = " << lcg.GenNext() << " = LCG value\n";

	return 0;
}