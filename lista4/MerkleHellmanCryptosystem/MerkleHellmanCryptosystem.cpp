// MerkleHellmanCryptosystem.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <iomanip>

#include "MerkleHellman.h"

int main()
{
	int success = 0;
	int n = 100;
	for (int i = 0; i < n; ++i)
	{
		std::cout << "i: " << i << "/" << n << std::endl;
		MerkleHellman merkle;
		merkle.GenerateKey();

		std::string msg = "wiadma104bity";
		mpz_class cipher = 0;
		merkle.Encrypt(msg, cipher.get_mpz_t());

		//std::cout << "encrypted: " << cipher << std::endl

		std::string plain = "";
		merkle.Decrypt(cipher.get_mpz_t(), plain);

		if (msg == plain)
			++success;
	}

	std::cout << "success: " << success << "/" << n << std::endl;
}

