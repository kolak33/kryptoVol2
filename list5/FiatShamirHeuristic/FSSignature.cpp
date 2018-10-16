#include "stdafx.h"
#include "FSSignature.h"

#include <openssl\sha.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <bitset>

FSSignature::FSSignature(int primeNumBits, std::string identificationId, int k, int t)
	: _primeNumBits(primeNumBits),
	_identificationId(identificationId),
	_k(k),
	_t(t)
{
	_p = _primeGen.GenerateRandomPrime(_primeNumBits);
	_q = _primeGen.GenerateRandomPrime(_primeNumBits);
	_n = _p * _q;

	_qrHolder.reserve(_k);

	CreateQuadraticResidues();
}

std::string FSSignature::Sha512Hash(std::string message)
{
	unsigned char hash[SHA512_DIGEST_LENGTH];
	SHA512((unsigned char*)message.c_str(), message.size(), (unsigned char*)&hash);

	std::stringstream buffer;
	for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
	{
		buffer << std::hex << std::setfill('0');
		buffer << std::setw(2) << static_cast<unsigned>(hash[i]);
	}
	std::string hexStr = buffer.str();
	return hexStr;
}

void FSSignature::CRT(mpz_t x, std::vector<mpz_ptr> v, std::vector<mpz_ptr> m) 
{
	const int t = 4;
	mpz_t u;
	mpz_t C[t];
	int i, j;

	mpz_init(u);
	for (i = 1; i<t; i++) {
		mpz_init(C[i]);
		mpz_set_ui(C[i], 1);
		for (j = 0; j<i; j++) {
			mpz_invert(u, m[j], m[i]);
			mpz_mul(C[i], C[i], u);
			mpz_mod(C[i], C[i], m[i]);
		}
	}
	mpz_set(u, v[0]);
	mpz_set(x, u);
	for (i = 1; i<t; i++) {
		mpz_sub(u, v[i], x);
		mpz_mul(u, u, C[i]);
		mpz_mod(u, u, m[i]);
		for (j = 0; j<i; j++) {
			mpz_mul(u, u, m[j]);
		}
		mpz_add(x, x, u);
	}

	for (i = 1; i<t; i++) mpz_clear(C[i]);
	mpz_clear(u);
}

void FSSignature::CreateQuadraticResidues()
{
	// generate quadratic residues v_j (mod n) for 1 ... k, and compute square root s of v^-1 (mod n)
	unsigned int correctIter = 0;
	unsigned int j = 1;
	do
	{
		std::string idTemp = _identificationId + std::to_string(j);
		std::string hashStr = Sha512Hash(idTemp);
		++j;

		mpz_class hashInt;
		mpz_set_str(hashInt.get_mpz_t(), hashStr.c_str(), 16); //base 16
	
		hashInt = (hashInt) % _n;

		mpz_class gcd;
		mpz_gcd(gcd.get_mpz_t(), hashInt.get_mpz_t(), _n.get_mpz_t());

		if (gcd != mpz_class(1))
			continue; // try next hash

		int isQRp = mpz_legendre(hashInt.get_mpz_t(), _p.get_mpz_t());
		int isQRq = mpz_legendre(hashInt.get_mpz_t(), _q.get_mpz_t());

		if (isQRp == -1 || isQRq == -1)
			continue;

		/*TEST, v_j must be a quadratic residue mod n*/
		/* so it is a residue mod p, and mod q */
		mpz_class qrTest = hashInt;// % _p;
		mpz_class testRes;
		mpz_class expPowerTest = (_p - 1) / 2;
		mpz_powm(testRes.get_mpz_t(), qrTest.get_mpz_t(), expPowerTest.get_mpz_t(), _p.get_mpz_t());
		std::cout << "testRes   : " << testRes << "\n";
		
		if ((testRes % _p) != 1)
			continue;

		// v^-1
		mpz_class quadrResidue = hashInt;
		mpz_invert(hashInt.get_mpz_t(), hashInt.get_mpz_t(), _n.get_mpz_t());

		if (((quadrResidue * hashInt) % _n) != 1)
			std::cout << "v * v^-1 != 1 (mod n)\n";

		isQRp = mpz_legendre(hashInt.get_mpz_t(), _p.get_mpz_t());
		isQRq = mpz_legendre(hashInt.get_mpz_t(), _q.get_mpz_t());

		if (isQRp == -1 || isQRq == -1)
			std::cout << "should not happen, tested before\n";

		// calculare square roots modulo primes
		mpz_class sqRootModP, sqRootModQ;
		mpz_class exponentP = (_p + 1) / 4;
		mpz_class exponentQ = (_q + 1) / 4;

		mpz_powm(sqRootModP.get_mpz_t(), hashInt.get_mpz_t(), exponentP.get_mpz_t(), _p.get_mpz_t());
		mpz_powm(sqRootModQ.get_mpz_t(), hashInt.get_mpz_t(), exponentQ.get_mpz_t(), _q.get_mpz_t());

		++correctIter;

		if ((_p % 4) != 3 || (_q % 4) != 3)
			std::cout << "p or q % 4 != 3\n";

		if(((sqRootModP * sqRootModP) % _p) - (hashInt % _p) != 0 || ((sqRootModQ * sqRootModQ) % _q) - (hashInt % _q) != 0)
			std::cout << "1. v - s^2 != 0\n";

		const int size = 4;
		std::vector<mpz_ptr> residuals, primes;
		residuals.resize(size); primes.resize(size);
		primes[0] = _p.get_mpz_t(); residuals[0] = sqRootModP.get_mpz_t();
		primes[1] = _p.get_mpz_t(); residuals[1] = mpz_class(sqRootModP * -1).get_mpz_t();
		primes[2] = _q.get_mpz_t(); residuals[2] = sqRootModQ.get_mpz_t();
		primes[3] = _q.get_mpz_t(); residuals[3] = mpz_class(sqRootModQ * -1).get_mpz_t();

		mpz_class squareRoot;
		CRT(squareRoot.get_mpz_t(), residuals, primes);

		QuadraticResidueHolder qr;
		qr.j = j-1;
		qr.quadraticResidueV = quadrResidue;
		qr.squareRootS = squareRoot % _n;

		//std::cout << "j = " << j << "qr = " << hashInt << "\ns = " << inverseS << "\n";
		_qrHolder.push_back(qr);

		if (((hashInt % _n) - (qr.squareRootS * qr.squareRootS) % _n) != 0)
			std::cout << "2. v^-1 - s^2 != 0\n";

	} while (correctIter < _k);

	std::cout << "Generated " << _k << "residues after " << j-1 << " iters.\n";
}

MessageSignature FSSignature::SignMessage(std::string message)
{
	// pick random r_1, ..., r_t values, then compute x_i = (r_i)^2 (mod n)
	std::vector<mpz_class> randVec;
	std::string hashInput = message;
	for (int i = 0; i < _t; ++i)
	{
		mpz_class rand = _primeGen.GenerateRandomNumber(_n);
		randVec.push_back(rand);
	
		mpz_class x = (rand * rand) % _n;
		hashInput += x.get_str();
	}
	// compute hash(m, x_1, ..., x_t) and use its first k*t bits as e_ij values
	std::string hashStr = Sha512Hash(hashInput);

	MessageSignature signature;
	signature.identificationId = _identificationId;
	signature.message = message;
	
	// get e_ij values
	for (int i = 0, charIter = 0; i < _k * _t; ++charIter)
	{
		std::bitset<8> bits(hashStr[charIter]);
		int bitIter = 0;
		while (bitIter < 8 && i < _k * _t)
		{
			signature.randomCoinsE.push_back(bits[bitIter]);
			++bitIter;
			++i;
		}
	}
	
	// calculate y values, y_i = r_i * ((PRODUCT over e_ij) s_j) (mod n)
	for (int i = 0; i < _t; ++i)
	{
		mpz_class y = randVec[i];
		for (int j = 0; j < _k; ++j)
		{
			if (signature.randomCoinsE[i*j + j] == true) // if e_ij = 1
				y = (y * _qrHolder[j].squareRootS) % _n; // y = y * s_j
		}
		signature.yVec.push_back(y);
	}

	for (int i = 0; i < _k; ++i)
		signature.usedJValues.push_back(_qrHolder[i].j);

	return signature;
}

bool FSSignature::VerifySignature(MessageSignature &signature)
{
	// compute v_j = hash(Id, j) for proper j
	std::vector<mpz_class> qrVec;

	for (int i = 0; i < _k; ++i)
	{
		std::string hashInput = signature.identificationId + std::to_string(signature.usedJValues[i]);
		std::string hashStr = Sha512Hash(hashInput);

		mpz_class hashInt;
		mpz_set_str(hashInt.get_mpz_t(), hashStr.c_str(), 16); //base 16
		hashInt = hashInt % _n;

		qrVec.push_back(hashInt);

		///if (qrVec[i].get_str() != _qrHolder[i].quadraticResidueV.get_str())
		///	std::cout << "wrong v (quadratic residue)\n";
	}

	// compute z_i = (y_i)^2 ((PRODUCT over e_ij=1) * v_j) (mod n) for i = 1, ..., t

	std::string hashInput = signature.message;
	for (int i = 0; i < _t; ++i)
	{
		mpz_class z = (signature.yVec[i] * signature.yVec[i]) % _n;
		for (int j = 0; j < _k; ++j)
		{
			if (signature.randomCoinsE[i * j + j] == true)
				z = (z * qrVec[j]) % _n;
		}
		hashInput += z.get_str();
	}

//	std::cout << "hashinp2: " << hashInput << "\n";
	std::string hashStr = Sha512Hash(hashInput);
	
	//Verify that first k*t bits are e_ij values
	for (int i = 0, charIter = 0; i < _k * _t; ++charIter)
	{
		std::bitset<8> bits(hashStr[charIter]);
		int bitIter = 0;
		while (bitIter < 8 && i < _k * _t)
		{
			if (bits[bitIter] != signature.randomCoinsE[i])
				return false;

			++bitIter;
			++i;
		}
	}

	return true; // all coins are equal as they should be
}

MessageSignature FSSignature::ForgeSignaturePreprocessing(std::string fakeID, std::string message)
{
	std::vector<mpz_class> randVecY;
	std::vector<unsigned int> randVecUsedJValues;

	MessageSignature fakeSignature;
	fakeSignature.message = message;
	fakeSignature.identificationId = fakeID;

	for (int i = 0; i < _t; ++i)
	{
		mpz_class rand = _primeGen.GenerateRandomNumber(_n);
		randVecY.push_back(rand);
	}

	for (int i = 0; i < _k; ++i)
	{
		randVecUsedJValues.push_back(i+1);
	}

	fakeSignature.yVec = randVecY;
	fakeSignature.usedJValues = randVecUsedJValues;

	return fakeSignature;
}

int FSSignature::ForgeSignatureAttack(std::string fakeID, std::string message)
{
	MessageSignature fakeSign = ForgeSignaturePreprocessing(fakeID, message);
	fakeSign.randomCoinsE.resize(_k * _t);

	//guess e_ij vector
	for (int i = 0; i < std::pow(2, _k * _t + 1); ++i)
	{
		int num = i;
		for (int j = 0; j < _k * _t; ++j)
		{
			if (num & 1)
				fakeSign.randomCoinsE[j] = true;
			else
				fakeSign.randomCoinsE[j] = false;
			num >>= 1;
		}
		if (VerifySignature(fakeSign))
			return i;
	}

	return -1;
}