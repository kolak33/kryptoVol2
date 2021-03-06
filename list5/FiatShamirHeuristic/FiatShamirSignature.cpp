// FiatShamirSignature.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <mpirxx.h>
#include <string>

#include "FSSignature.h"
#include <iostream>

int main()
{
	//std::string message = "to jest moja wiadomosc do podpisu";
	//uint32_t hash = HashFNV((const void *)(message.c_str()), message.size());

	int k = 3, t = 4;
	std::string id = "AlicjaId30";
	FSSignature fsSign(256, id, k, t);

	int test = 100;
	int correct = 0;
	for (int i = 0; i < test; ++i)
	{
		MessageSignature msgSignature = fsSign.SignMessage("message to be signed");
		correct += fsSign.VerifySignature(msgSignature);
		//std::cout << "signature is: " << (fsSign.VerifySignature(msgSignature) ? "correct\n" : "wrong\n");
	}
	std::cout << "correct: " << correct << "/" << test << "\n";

	std::string msg = "fakemsg";
	
	int testAttack = 1000;
	int success = 0;
	long long allTries = 0;
	for (int i = 0; i < testAttack; ++i)
	{
		std::cout << "i: " << i << "\n";
		int numTries = fsSign.ForgeSignatureAttack(id, msg);
		if (numTries != -1)
		{
			++success;
			allTries += numTries;
		}
	}
	std::cout << "success: " << ((double)success / (double)testAttack) * 100.0 << "%\n";
	std::cout << "average guessess needed: " << allTries / (long long)success << "\n";
	std::cout << "maxGuesses: " << std::pow(2, k * t) << "\n";

    return 0;
}

