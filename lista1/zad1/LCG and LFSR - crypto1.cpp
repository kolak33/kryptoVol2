// LCG and LFSR - crypto1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <atldef.h>
#include <vector>
#include <algorithm>

#include "LCG.h"
#include "LCGBreaker.h"

/*#define MAX 400
#define seed 1
srand(1L);
int r[MAX];
int i;
r[0] = seed;
for (i = 1; i<31; i++) {
r[i] = (16807LL * r[i - 1]) % 2147483647;
if (r[i] < 0) {
r[i] += 2147483647;
}
}
for (i = 31; i<34; i++) {
r[i] = r[i - 31];
}
for (i = 34; i<344; i++) {
r[i] = r[i - 31] + r[i - 3];
}
for (i = 344; i<MAX; i++) {
r[i] = r[i - 31] + r[i - 3];
printf("%d =?= %d\n", ((unsigned int)r[i]) >> 1, random());
}*/


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

	for(int i=0; i<10; ++i)
		std::cout << "prediction = " << lcgBreaker.PredictNext() << " = " << lcg.GenNext() << " = LCG value\n";

	return 0;
}