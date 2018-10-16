#pragma once

#include "RandomNumberGenerator.h"
#include <mpirxx.h>
#include <string>
#include <vector>

class QuadraticResidueHolder
{
public:
	mpz_class quadraticResidueV;
	mpz_class squareRootS;
	unsigned int j;
};

class MessageSignature
{
public:
	std::string identificationId; // who signed the message
	std::string message;
	std::vector<bool> randomCoinsE; // e_ij values, k * t random bits from hash: f(m, x_1, ..., x_t)
	std::vector<mpz_class> yVec; // y_i = r_i * ((PRODUCT OVER e_ij = 1) s_j) mod n
	std::vector<unsigned int> usedJValues;
};

class FSSignature
{
public:
	FSSignature(int primeNumBits, std::string identificationId, int k, int t);

	MessageSignature SignMessage(std::string message);
	bool VerifySignature(MessageSignature &signature);
	MessageSignature ForgeSignaturePreprocessing(std::string fakeID, std::string message);
	int ForgeSignatureAttack(std::string fakeID, std::string message);
private:
	mpz_class _p, _q; // primes
	mpz_class _n; // n = p * q
	int _k, _t;
	int _primeNumBits;
	std::vector<QuadraticResidueHolder> _qrHolder;
	RandomNumberGenerator _primeGen;

	void CreateQuadraticResidues();
	std::string Sha512Hash(std::string message);
	void CRT(mpz_t x, std::vector<mpz_ptr> v, std::vector<mpz_ptr> m);

private:
	std::string _identificationId; // string which contains all
								 // relevant information about the user: address, name, Id number etc.,
};

