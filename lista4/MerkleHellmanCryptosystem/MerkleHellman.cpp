#include "stdafx.h"
#include "MerkleHellman.h"

#include <iostream>
#include <ctime>
#include <string>
#include <bitset>

MerkleHellman::MerkleHellman()
{
	SeedRandomState();
}

MerkleHellman::~MerkleHellman()
{
	gmp_randclear(m_rstate);
	delete[] m_superIncreasingW;
	delete[] m_B;
}

void MerkleHellman::SeedRandomState()
{
	gmp_randinit_mt(m_rstate);
	gmp_randseed_ui(m_rstate, time(nullptr));
}

/* numbers between [(2^(i-1) - 1) * 2^100; 2^(i-1) * 2^100] */
void MerkleHellman::CreateSuperIncreasingSequenceW(int numBits)
{
	m_superIncreasingW = new mpz_class[numBits];

	mpz_class boundRand;
	mpz_pow_ui(boundRand.get_mpz_t(), mpz_class(2).get_mpz_t(), numBits); // 2 ^ n

	for (int i = 1; i <= numBits; ++i)
	{
		mpz_class rand;
		mpz_urandomb(rand.get_mpz_t(), m_rstate, numBits); // random [0, 2^n - 1]

		/* left bound = (2^(i-1) - 1) * 2^n */
		mpz_class leftBound;
		mpz_pow_ui(leftBound.get_mpz_t(), mpz_class(2).get_mpz_t(), i - 1); // 2^(i-1)
		leftBound--;

		leftBound = leftBound * boundRand;
		rand += leftBound;

		m_superIncreasingW[i - 1] = rand;
	}

	/* TEST, whether M is superincreasing */
	mpz_class sum = 0;
	for (int i = 0; i < numBits - 1; ++i)
	{
		sum += m_superIncreasingW[i];
		if(sum >= m_superIncreasingW[i + 1])
			std::cout << "!!!!!!!!!!!!!!!!!!!!! NOT SUPERINCREASING !!!!!!!!!!!!!!!!!!!!!\n";
	}
}

/* create modulus M, random number between [2^201 + 1, 2^202 - 1]*/
void MerkleHellman::CreateModulusM(int numBits)
{
	mpz_class boundRand;
	mpz_pow_ui(boundRand.get_mpz_t(), mpz_class(2).get_mpz_t(), ((2*numBits) + 30)); // +30 because M was too low after division by gcd
	boundRand--;

	mpz_class rand;
	mpz_urandomm(rand.get_mpz_t(), m_rstate, boundRand.get_mpz_t()); // random [0, 2^201 - 2]

	// rand + 2^201 + 1, so that our interval is [2^201 + 1, 2^202 - 1]
	rand += boundRand;
	rand += 2;

	m_modulusM = rand;

	/* TEST, whether M is superincreasing */
	mpz_class sum = 0;
	for (int i = 0; i < numBits; ++i)
	{
		sum += m_superIncreasingW[i];
	}

	if(sum >= m_modulusM)
		std::cout << "!!!!!!!!!!!!!!!!!!!!! M IS LOWER THAN SUM OF SUPERINCREASING !!!!!!!!!!!!!!!!!!!!!\n";
}

/* choose R' uniformly from [2, M-2] and then divide by the greatest common divisor of R' and M to yield R */
void MerkleHellman::CreateMultiplierR()
{
	mpz_class multRTemp = m_modulusM; // random [0; M - 4]
	multRTemp -= 4;

	mpz_urandomm(multRTemp.get_mpz_t(), m_rstate, multRTemp.get_mpz_t()); // random [0, M - 4]
	multRTemp += 2; // [2, M - 2]

	/* compute GCD(R', M) */
	mpz_class gcdDivisor;
	mpz_gcd(gcdDivisor.get_mpz_t(), m_modulusM.get_mpz_t(), multRTemp.get_mpz_t());

	multRTemp /= gcdDivisor;
	m_modulusM /= gcdDivisor;

	m_multR = multRTemp;

	mpz_class sum = 0;
	for (int i = 0; i < m_WSize; ++i)
	{
		sum += m_superIncreasingW[i];
	}
	if(sum >= m_modulusM)
		std::cout << "!!!!!!!!!!!!!!!!!!!!! M IS LOWER THAN SUM OF SUPERINCREASING !!!!!!!!!!!!!!!!!!!!!\n";

	
}

void MerkleHellman::CreatePublicKey()
{
	m_B = new mpz_class[m_WSize];

	for (int i = 0; i < m_WSize; ++i)
	{
		m_B[i] = m_multR * m_superIncreasingW[i];
		m_B[i] %= m_modulusM;
	}
}

void MerkleHellman::GenerateKey(int numBits)
{
	m_WSize = numBits;
	CreateSuperIncreasingSequenceW(numBits);
	CreateModulusM(numBits);
	CreateMultiplierR();
	CreatePublicKey();
}

/* encrypt n-bit blocks, n = 104 for easier computations */
void MerkleHellman::Encrypt(std::string &message, mpz_t ciphertext)
{
	// sum over {0..n}bits of msg: cipher += msg[i] * B[i]

	mpz_class sum = 0;
	for (size_t i = 0; i < message.length(); ++i)
	{
		auto msgByte = std::bitset<8>(message.c_str()[i]);
		for (size_t j = 0; j < msgByte.size(); ++j)
		{
			int bit = msgByte[j];
			mpz_class temp;
			temp = m_B[(i * 8) + j] * bit;
			mpz_add(ciphertext, ciphertext, temp.get_mpz_t());
		}
	}
}

void MerkleHellman::Decrypt(mpz_t ciphertext, std::string &message)
{
	// calc inverse of R
	mpz_class rInverse;
	mpz_invert(rInverse.get_mpz_t(), m_multR.get_mpz_t(), m_modulusM.get_mpz_t());

	mpz_class sol;
	mpz_mul(sol.get_mpz_t(), ciphertext, rInverse.get_mpz_t());
	sol %= m_modulusM;

	std::string msgBits = "";
	for (int WIter = m_WSize - 1; WIter >= 0; --WIter)
	{
		if(sol >= m_superIncreasingW[WIter] && sol > 0)
		{
			sol -= m_superIncreasingW[WIter];
			msgBits += '1';
		}
		else
			msgBits += '0';
	}

	std::reverse(msgBits.begin(), msgBits.end());

	message = "";
	for (int i = 0; i < msgBits.size() / 8; ++i)
	{
		std::string substr = msgBits.substr(i * 8, 8);
		std::reverse(substr.begin(), substr.end());
		message += (char)std::bitset<8>(substr).to_ulong();
	}

	std::cout << "decoded: " << message << std::endl;
}