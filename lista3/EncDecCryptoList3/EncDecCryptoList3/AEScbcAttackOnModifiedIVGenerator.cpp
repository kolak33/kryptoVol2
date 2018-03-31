#include "stdafx.h"
#include "AEScbcAttackOnModifiedIVGenerator.h"

#include <openssl/rand.h>
#include <iostream>
#include <random>
#include <ctime>

AEScbcAttackOnModifiedIVGenerator::AEScbcAttackOnModifiedIVGenerator(std::string &strPathToTheKeyStore, std::string &strKeystorePassword,
	std::string &strKeyIdentifier, std::string &strKeyPassword)
	: AES256(strPathToTheKeyStore, strKeystorePassword, strKeyIdentifier, strKeyPassword)
{
	bool bAllocatedIV = false;
	unsigned char *newIV = nullptr;
	__super::GenerateRandomIV(&newIV, bAllocatedIV);

	m_lastIV = newIV;
	srand(time(NULL));
}

AEScbcAttackOnModifiedIVGenerator::AEScbcAttackOnModifiedIVGenerator()
{
	bool bAllocatedIV = false;
	unsigned char *newIV = nullptr;
	__super::GenerateRandomIV(&newIV, bAllocatedIV);

	m_lastIV = newIV;
	srand(time(NULL));
}


AEScbcAttackOnModifiedIVGenerator::~AEScbcAttackOnModifiedIVGenerator()
{
}

bool AEScbcAttackOnModifiedIVGenerator::GenerateRandomIV(unsigned char **iv, bool &bOutAllocated)
{
	if (*iv == nullptr)
	{
		*iv = new unsigned char[m_iIVSize]; // 128bits block

		if (m_lastIV == nullptr)
			std::cout << "last IV not generated\n";

		unsigned char *tempValue = new unsigned char[m_iIVSize];
		std::memcpy(tempValue, m_lastIV, m_iIVSize);

		for (int i = 0; i < m_iIVSize; ++i)
		{
			if (++tempValue[m_iIVSize - 1 - i] != 0)
				break;
		}

		std::memcpy((*iv), tempValue, m_iIVSize); 
		std::memcpy(m_lastIV, tempValue, m_iIVSize); 
		//std::memcpy((*iv), m_lastIV, m_iIVSize); //temp
		//RAND_bytes(*iv, sizeof(*iv));
		bOutAllocated = true;

		delete[]tempValue;
	}
	return true;
}

void AEScbcAttackOnModifiedIVGenerator::IncrementIV(unsigned char **iv)
{
	unsigned char *tempValue = new unsigned char[m_iIVSize];
	std::memcpy(tempValue, m_lastIV, m_iIVSize);

	for (int i = 0; i < m_iIVSize; ++i)
	{
		if (++tempValue[m_iIVSize - 1 - i] != 0)
			break;
	}
	std::memcpy((*iv), tempValue, m_iIVSize);
	delete[]tempValue;
}

int AEScbcAttackOnModifiedIVGenerator::CPAAttack()
{
	// first, get te IV by sending random message
	unsigned char *plain1 = new unsigned char[m_iIVSize];
	for (int i = 0; i < m_iIVSize; ++i)
		plain1[i] = (unsigned char)(i+60);

	unsigned char *cipher1 = nullptr;// new unsigned char[CalcCiphertextLen(m_iIVSize)];
	int cipher1Len = 0;
	unsigned char *firstIV = nullptr;
	EncryptMessage(plain1, m_iIVSize, EVP_aes_256_cbc(), &cipher1, &cipher1Len, &firstIV);

	unsigned char *decrypted1 = nullptr;// = new unsigned char[m_iIVSize];
	int decrypted1Len = 0;
	DecryptMessage(cipher1, cipher1Len, EVP_aes_256_cbc(), firstIV, &decrypted1, &decrypted1Len);

	// now we know what iv will be used in next encryption (iv + 1)
	IncrementIV(&firstIV);

	//now send new plaintext that is equal to (iv + 1), ciphertext will be F_k(0^n)
	unsigned char *plain2 = new unsigned char[m_iIVSize];
	std::memcpy(plain2, firstIV, m_iIVSize);
	unsigned char *cipherIncrementedIV = nullptr;// = new unsigned char[CalcCiphertextLen(m_iIVSize)];
	int cipher2Len = 0;
	unsigned char *secondIV = nullptr;
	EncryptMessage(plain2, m_iIVSize, EVP_aes_256_cbc(), &cipherIncrementedIV, &cipher2Len, &secondIV);

	unsigned char *decrypted2 = nullptr;// = new unsigned char[m_iIVSize];
	int decrypted2Len = 0;
	DecryptMessage(cipherIncrementedIV, cipher2Len, EVP_aes_256_cbc(), secondIV, &decrypted2, &decrypted2Len);

	//now we can send to oracle random message and message equal to incremented IV, 
	//if output is F_k(0^n) then message sent was incremented IV, otherwise random message

	unsigned char *cipherOracle = nullptr;// = new unsigned char[CalcCiphertextLen(m_iIVSize)];
	int cipherOracleLen = 0;

	IncrementIV(&secondIV);
	unsigned char *firstMessage = new unsigned char[m_iIVSize];
	std::memcpy(firstMessage, secondIV, m_iIVSize);

	unsigned char *secondMessage = new unsigned char[m_iIVSize];
	RAND_bytes(secondMessage, m_iIVSize);

	int bitOracle = SendToOracle(firstMessage, m_iIVSize, secondMessage, m_iIVSize, &cipherOracle, &cipherOracleLen);
	int bitAdversary;
	if (std::strncmp((char*)cipherOracle, (char*)cipherIncrementedIV, cipherOracleLen) == 0)
	{
		//std::cout << "First message was sent (incremented IV)\n";
		bitAdversary = 0;
	}
	else
	{
		//std::cout << "Second message was sent (random msg)\n";
		bitAdversary = 1;
	}

	delete[]plain1;
	delete[]cipher1;
	delete[]decrypted1;
	delete[]plain2;
	delete[]cipherIncrementedIV;
	delete[]firstIV;
	delete[]secondIV;
	delete[]decrypted2;
	delete[]cipherOracle;
	delete[]firstMessage;
	delete[]secondMessage;

	return (bitOracle == bitAdversary);
}

void AEScbcAttackOnModifiedIVGenerator::TestAttack()
{
	int numTests = 500;
	int correct = 0;
	for (int i = 0; i < numTests; ++i)
	{
		if (CPAAttack())
			correct++;
	}
	std::cout << "Attack correct in " << correct << "/" << numTests << " times.\n";
}

int AEScbcAttackOnModifiedIVGenerator::SendToOracle(unsigned char *strMessage1, int strMessage1Len, unsigned char *strMessage2, 
	int strMessage2Len, unsigned char **encryptedMessage, int *encryptedMsgLen)
{
	unsigned int rand = std::rand();

	if (m_key == nullptr)
	{
		std::cout << "Key has not been loaded" << std::endl;
		return -1;
	}

	if (strMessage1Len != strMessage2Len)
	{
		std::cout << "Messages must have the same length" << std::endl;
		return -1;
	}

	unsigned char *ivtemp;
	unsigned char *msgToEncrypt;
	int retVal;
	if (rand & 1)
	{
		msgToEncrypt = strMessage1;
		//std::cout << "ORACLE WILL CIPHER first message, (incremented IV)\n";
		retVal = 0;
	}
	else
	{
		msgToEncrypt = strMessage2;
		//std::cout << "ORACLE WILL CIPHER second message, (random msg)\n";
		retVal = 1;
	}

	EncryptMessage(msgToEncrypt, strMessage1Len, EVP_aes_256_cbc(), encryptedMessage, encryptedMsgLen, &ivtemp);
	return retVal;
}