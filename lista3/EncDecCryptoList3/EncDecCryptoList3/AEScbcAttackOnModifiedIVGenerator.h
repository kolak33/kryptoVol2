#pragma once

#include "AES256.h"

class AEScbcAttackOnModifiedIVGenerator : public AES256
{
public:
	AEScbcAttackOnModifiedIVGenerator();
	~AEScbcAttackOnModifiedIVGenerator();

	AEScbcAttackOnModifiedIVGenerator(std::string &strPathToTheKeyStore, std::string &strKeystorePassword,
		std::string &strKeyIdentifier, std::string &strKeyPassword);

	void CPAAttack();

protected:
	virtual bool GenerateRandomIV(unsigned char **iv, bool &bOutAllocated);
	void IncrementIV(unsigned char **iv);

	void SendToOracle(unsigned char *strMessage1, int strMessage1Len, unsigned char *strMessage2, 
		int strMessage2Len, unsigned char **encryptedMessage, int *encryptedMsgLen);

	unsigned char* m_lastIV;
};

