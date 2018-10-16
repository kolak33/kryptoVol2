#include "stdafx.h"
#include "RandomNumberGenerator.h"

#include <ctime>

RandomNumberGenerator::RandomNumberGenerator()
{
	SeedRandomState();
}


RandomNumberGenerator::~RandomNumberGenerator()
{
	gmp_randclear(m_rstate);
}

void RandomNumberGenerator::SeedRandomState()
{
	gmp_randinit_mt(m_rstate);
	gmp_randseed_ui(m_rstate, time(nullptr));
}

mpz_class RandomNumberGenerator::GenerateRandomPrime(int numBits)
{
	mpz_class rand;

	do {
		mpz_urandomb(rand.get_mpz_t(), m_rstate, numBits);
	} while (mpz_probab_prime_p(rand.get_mpz_t(), 50) == 0 || ((rand % 4) != 3));

	return rand;
}

mpz_class RandomNumberGenerator::GenerateRandomNumber(int numBits)
{
	mpz_class rand;
	mpz_urandomb(rand.get_mpz_t(), m_rstate, numBits);
	return rand;
}

mpz_class RandomNumberGenerator::GenerateRandomNumber(mpz_class n)
{
	mpz_class rand;
	mpz_urandomm(rand.get_mpz_t(), m_rstate, n.get_mpz_t()); // rand [0, n-1]
	return rand;
}