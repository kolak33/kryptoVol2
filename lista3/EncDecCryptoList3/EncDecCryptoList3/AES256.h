#pragma once

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
//#include <openssl/applink.c>
#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string>
#include <vector>

#include <chilkat\include\CkJavaKeyStore.h>
#include <chilkat\include\CkCert.h>
#include <chilkat\include\CkPrivateKey.h>
#include <chilkat\include\CkCertChain.h>
#include <chilkat\include\C_CkCert.h>
#include <chilkat\include\CkGlobal.h>

class AES256
{
public:
	AES256();
	~AES256();

	AES256(std::string &strPathToTheKeyStore, std::string &strKeystorePassword,
		std::string &strKeyIdentifier, std::string &strKeyPassword);
	
	bool EncryptFile(std::string &strFileName, std::string &strPathToTheKeyStore, std::string &strKeystorePassword,
		std::string &strKeyIdentifier, std::string &strKeyPassword, const EVP_CIPHER *cipher_mode, unsigned char *iv = nullptr);

	bool DecryptFile(std::string &strFileName, std::string &strPathToTheKeyStore, std::string &strKeystorePassword,
		std::string &strKeyIdentifier, std::string &strKeyPassword, const EVP_CIPHER *cipher_mode);

	bool EncryptMessage(unsigned char* strMessage, int strMessageLen, const EVP_CIPHER *cipher_mode,
		unsigned char **encryptedMessage, int *encryptedMsgLen, unsigned char **outIV);

	bool DecryptMessage(unsigned char *strMessage, int strMessageLen, const EVP_CIPHER *cipher_mode,
		unsigned char *iv, unsigned char **decryptedMessage, int *decryptedMsgLen);

	void ReadKey(std::string &strPathToTheKeyStore, std::string &strKeystorePassword,
		std::string &strKeyIdentifier, std::string &strKeyPassword);
protected:
	virtual bool GenerateRandomIV(unsigned char **iv, bool &bOutAllocated);
	unsigned int CalcCiphertextLen(unsigned int plaintextLen);

	int m_iKeySizeInBytes;
	int m_iIVSize;
	unsigned char *m_key;
private:
	void HandleErrors(void);
	bool LoadKeystore(std::string &strPathToTheKeyStore, std::string &strKeystorePassword, CkJavaKeyStore &outJceks);
	bool LoadSecretKey(CkJavaKeyStore &jceks, std::string &strSecretKeyAlias, std::string &strKeyPassword, std::string &outSecretKey);
	int FindSecretKeyPosition(CkJavaKeyStore &jceks, std::string &strSecretKeyAlias);
	bool SaveCipherToFile(unsigned char *ciphertext, int ciphertextLen, unsigned char *iv, std::string &strFileName);
	bool SavePlainToFile(unsigned char *plaintext, int plaintextLen, std::string &strFileName);
	
	int Encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
		unsigned char *iv, unsigned char *ciphertext, const EVP_CIPHER *cipher_mode);

	int Decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
		unsigned char *iv, unsigned char *plaintext, const EVP_CIPHER *cipher_mode);
};

