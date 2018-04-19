#pragma once

#pragma warning(disable: 4800)
#include <mpirxx.h>
#pragma warning(disable: 4800)
#include <vector>

class MerkleHellman
{
public:
	MerkleHellman();
	~MerkleHellman();

	void GenerateKey(int numBits = 104);
	void Encrypt(std::string &message, mpz_t ciphertext);
	void Decrypt(mpz_t ciphertext, std::string &message);

protected:
	void SeedRandomState();
	void CreateSuperIncreasingSequenceW(int numBits);
	void CreateModulusM(int numBits);
	void CreateMultiplierR();
	void CreatePublicKey();

private:
	/* private key W, M, R */
	mpz_class *m_superIncreasingW;
	int m_WSize;
	mpz_class m_modulusM;
	mpz_class m_multR;

	/* public key B */
	mpz_class *m_B;

private:
	gmp_randstate_t m_rstate;
};

