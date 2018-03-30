#include "stdafx.h"
#include "AES256.h"

#include <fstream>
#include <iostream>
#include <openssl\rand.h>
#include <boost/algorithm/string/replace.hpp>
#include <cstring>
#include "FileReader.h"

AES256::AES256()
	: m_iKeySizeInBytes(32)
	, m_iIVSize(16)
{
}


AES256::~AES256()
{
}

void AES256::HandleErrors(void)
{
	ERR_print_errors_fp(stderr);
	abort();
}

int AES256::Encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
	unsigned char *iv, unsigned char *ciphertext, const EVP_CIPHER *cipher_mode)
{
	EVP_CIPHER_CTX *ctx;
	int len;
	int ciphertext_len;

	/* Create and initialise the context */
	if (!(ctx = EVP_CIPHER_CTX_new())) HandleErrors();

	/* Initialise the encryption operation. IMPORTANT - ensure you use a key
	* and IV size appropriate for your cipher
	* In this example we are using 256 bit AES (i.e. a 256 bit key). The
	* IV size for *most* modes is the same as the block size. For AES this
	* is 128 bits */
	if (1 != EVP_EncryptInit_ex(ctx, cipher_mode, NULL, key, iv))
		HandleErrors();
	//EVP_PADDING_PKCS7

	/* Provide the message to be encrypted, and obtain the encrypted output.
	* EVP_EncryptUpdate can be called multiple times if necessary
	*/
	if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
		HandleErrors();
	ciphertext_len = len;

	/* Finalise the encryption. Further ciphertext bytes may be written at
	* this stage.
	*/
	if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) HandleErrors();
	ciphertext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

	return ciphertext_len;
}

int AES256::Decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
	unsigned char *iv, unsigned char *plaintext, const EVP_CIPHER *cipher_mode)
{
	EVP_CIPHER_CTX *ctx;
	int len;
	int plaintext_len;

	/* Create and initialise the context */
	if (!(ctx = EVP_CIPHER_CTX_new())) HandleErrors();

	/* Initialise the decryption operation. IMPORTANT - ensure you use a key
	* and IV size appropriate for your cipher
	* In this example we are using 256 bit AES (i.e. a 256 bit key). The
	* IV size for *most* modes is the same as the block size. For AES this
	* is 128 bits */
	if (1 != EVP_DecryptInit_ex(ctx, cipher_mode, NULL, key, iv))
		HandleErrors();
	//EVP_CIPHER_CTX_set_padding(ctx, 0);
	EVP_CIPHER_CTX_set_padding(ctx, EVP_PADDING_PKCS7); //TODO jakby cos nie dzialalo to zmienic na wczesniejsze

														/* Provide the message to be decrypted, and obtain the plaintext output.
														* EVP_DecryptUpdate can be called multiple times if necessary
														*/
	if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
		HandleErrors();
	plaintext_len = len;

	/* Finalise the decryption. Further plaintext bytes may be written at
	* this stage.
	*/
	if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) HandleErrors();
	plaintext_len += len;

	/* Clean up */
	EVP_CIPHER_CTX_free(ctx);

	return plaintext_len;
}

bool AES256::LoadKeystore(std::string &strPathToTheKeyStore, std::string &strKeystorePassword, CkJavaKeyStore &outJceks)
{
	bool success = outJceks.LoadFile(strKeystorePassword.c_str(), strPathToTheKeyStore.c_str());
	if (success != true)
		std::cout << outJceks.lastErrorText() << "\r\n";
	return success;
}

bool AES256::LoadSecretKey(CkJavaKeyStore &jceks, std::string &strSecretKeyAlias, std::string &strKeyPassword, std::string &outSecretKey)
{
	int secretKeyPos = FindSecretKeyPosition(jceks, strSecretKeyAlias);
	if (secretKeyPos == -1)
		return false;

	CkString ckstrSecretKey;
	jceks.GetSecretKey(strKeyPassword.c_str(), secretKeyPos, "base64", ckstrSecretKey);
	if (ckstrSecretKey.isEmpty())
	{
		std::cout << "Wrong password for key: " << strSecretKeyAlias << std::endl;
		outSecretKey = "";
		return false;
	}

	outSecretKey = ckstrSecretKey.getString();
	return true;
}

int AES256::FindSecretKeyPosition(CkJavaKeyStore &jceks, std::string &strSecretKeyAlias)
{
	CkString ckSecretKeyAlias;
	ckSecretKeyAlias.setString(strSecretKeyAlias.c_str());
	for (int i = 0; i < jceks.get_NumSecretKeys(); ++i)
	{
		CkString ckstrAlias;
		jceks.GetSecretKeyAlias(i, ckstrAlias);
		if (ckstrAlias.compareStr(ckSecretKeyAlias) == 1)
			return i;
	}
	std::cout << "Can't find secret key alias: " << strSecretKeyAlias << std::endl;
	return -1;
}

bool AES256::GenerateRandomIV(unsigned char **iv, bool &bOutAllocated)
{
	if (*iv == nullptr)
	{
		*iv = new unsigned char[m_iIVSize]; // 128bits block
		RAND_poll();
		if (RAND_bytes(*iv, sizeof(*iv)) != 1)
		{
			(*iv)[0] = (unsigned char)'k'; (*iv)[1] = (unsigned char)'u'; (*iv)[2] = (unsigned char)'b'; (*iv)[3] = (unsigned char)'a';
			for (int i = 4; i < m_iIVSize; ++i)
			{
				(*iv)[i] = (unsigned char)(rand() % 256);
			}
			std::cout << "Error while generating random IV, generated low quality IV" << std::endl;
			return false;
		}
		bOutAllocated = true;
	}
	return true;
}

bool AES256::SaveCipherToFile(unsigned char *ciphertext, int ciphertextLen, unsigned char *iv, std::string &strFileName)
{
	std::string strFile = boost::replace_all_copy(strFileName, "Dec.txt", "");
	strFile = boost::replace_all_copy(strFile, ".txt", "");
	strFile += "Enc.txt";

	std::ofstream file(strFile, std::ios::binary | std::ios::ate);
	if (!file.good())
		return false;

	file.write(reinterpret_cast<const char*>(iv), m_iIVSize);
	file.write(reinterpret_cast<const char*>(ciphertext), ciphertextLen);

	return true;
}

bool AES256::SavePlainToFile(unsigned char *plaintext, int plaintextLen, std::string &strFileName)
{
	std::string strFile = boost::replace_all_copy(strFileName, "Enc.txt", "");
	strFile = boost::replace_all_copy(strFile, ".txt", "");
	strFile += "Dec.txt";

	std::ofstream file(strFile, std::ios::binary | std::ios::ate);
	if (!file.good())
		return false;

	file.write(reinterpret_cast<const char*>(plaintext), plaintextLen);

	return true;
}

unsigned int AES256::CalcCiphertextLen(unsigned int plaintextLen)
{
	unsigned int blockSize = 16; //in bytes
	unsigned int paddingLen = plaintextLen % blockSize > 0 ? blockSize - plaintextLen % blockSize : 0; // padding in last 16 byte block

	return plaintextLen + paddingLen;
}

bool AES256::EncryptFile(std::string &strFileName, std::string &strPathToTheKeyStore, std::string &strKeystorePassword,
	std::string &strKeyIdentifier, std::string &strKeyPassword, const EVP_CIPHER *cipher_mode, unsigned char *iv)
{
	CkJavaKeyStore jceks;
	if (!LoadKeystore(strPathToTheKeyStore, strKeyPassword, jceks))
		return false;

	std::string strSecretKey;
	if (!LoadSecretKey(jceks, strKeyIdentifier, strKeyPassword, strSecretKey))
		return false;

	FileReader reader;
	std::vector<char> rawFile = reader.ReadAllBytes(strFileName.c_str());
	unsigned char *plaintext = reinterpret_cast<unsigned char*>(rawFile.data());

	bool bAllocatedIV = false;
	unsigned char *newIV = iv;
	if (!GenerateRandomIV(&newIV, bAllocatedIV))
		return false;
	if (newIV == nullptr)
		return false;

	unsigned int ciphertextLenCalc = CalcCiphertextLen(rawFile.size());
	unsigned char *ciphertext = new unsigned char[ciphertextLenCalc];
	if (ciphertext == nullptr)
	{
		std::cout << "Can't allocate memory for ciphertext" << std::endl;
		if (bAllocatedIV)
			delete []newIV;
		return false;
	}

	int ciphertextLen = Encrypt(plaintext, rawFile.size(), (unsigned char*)strSecretKey.c_str(), newIV, ciphertext, cipher_mode);

	bool bSavedToFile = SaveCipherToFile(ciphertext, ciphertextLen, newIV, strFileName);

	if (bAllocatedIV)
		delete []newIV;
	delete []ciphertext;

	return bSavedToFile;
}

bool AES256::DecryptFile(std::string &strFileName, std::string &strPathToTheKeyStore, std::string &strKeystorePassword,
	std::string &strKeyIdentifier, std::string &strKeyPassword, const EVP_CIPHER *cipher_mode)
{
	CkJavaKeyStore jceks;
	if (!LoadKeystore(strPathToTheKeyStore, strKeyPassword, jceks))
		return false;

	std::string strSecretKey;
	if (!LoadSecretKey(jceks, strKeyIdentifier, strKeyPassword, strSecretKey))
		return false;

	FileReader reader;
	std::vector<char> rawFile = reader.ReadAllBytes(strFileName.c_str());
	unsigned char *ciphertext = reinterpret_cast<unsigned char*>(rawFile.data());

	unsigned char *iv = new unsigned char[m_iIVSize];
	std::memcpy(iv, ciphertext, m_iIVSize);

	ciphertext += (m_iIVSize); // omit iv

	int ciphertextLen = rawFile.size() - m_iIVSize;
	unsigned char *plaintext = new unsigned char[ciphertextLen];
	if (plaintext == nullptr)
	{
		std::cout << "Can't allocate memory for plaintext" << std::endl;
		delete []iv;
		return false;
	}

	int plaintextLen = Decrypt(ciphertext, ciphertextLen, (unsigned char*)strSecretKey.c_str(), iv, plaintext, cipher_mode);

	bool bSavedToFile = SavePlainToFile(plaintext, plaintextLen, strFileName);

	delete[]iv;
	delete[]plaintext;

	return bSavedToFile;
}