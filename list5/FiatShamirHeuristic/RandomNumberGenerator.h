#pragma once

#include <mpirxx.h>

class RandomNumberGenerator
{
public:
	RandomNumberGenerator();
	~RandomNumberGenerator();

	mpz_class GenerateRandomPrime(int numBits);
	mpz_class GenerateRandomNumber(int numBits);
	mpz_class GenerateRandomNumber(mpz_class n);

private:
	void SeedRandomState();

private:
	gmp_randstate_t m_rstate;
};

