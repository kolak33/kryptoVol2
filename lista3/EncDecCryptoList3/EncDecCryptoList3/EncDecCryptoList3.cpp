// EncDecCryptoList3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
//#include <openssl/applink.c>
#include <openssl/buffer.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "Base64.h"
#include "AES256.h"

#include "AEScbcAttackOnModifiedIVGenerator.h"

//#include <chilkat\include\CkJavaKeyStore.h>
//#include <chilkat\include\CkCert.h>
//#include <chilkat\include\CkPrivateKey.h>
//#include <chilkat\include\CkCertChain.h>
//#include <chilkat\include\C_CkCert.h>
//#include <chilkat\include\CkGlobal.h>
#include <string.h>
//#include <stdio.h>
#include <iostream>
#include "FileReader.h"

bool InitLibraries()
{
	ERR_load_crypto_strings();
	OpenSSL_add_all_algorithms();
	OPENSSL_config(NULL);

	CkGlobal glob;
	bool success = glob.UnlockBundle("Anything for 30-day trial");
	if (success != true) {
		std::cout << glob.lastErrorText() << "\r\n";
	}
	return success;
}

bool SingleTest(std::string &strFileName, std::string &strPathToTheKeyStore, std::string &strKeystorePassword,
	std::string &strKeyIdentifier, std::string &strKeyPassword, const EVP_CIPHER *cipher_mode, std::string &strFileEncrypted, std::string &strFileDecrypted)
{
	AES256 aes;
	aes.EncryptFile(strFileName, strPathToTheKeyStore, strKeystorePassword, strKeyIdentifier, strKeyPassword, cipher_mode);

	aes.DecryptFile(strFileEncrypted, strPathToTheKeyStore, strKeystorePassword, strKeyIdentifier, strKeyPassword, cipher_mode);

	FileReader reader;
	unsigned int decrLen;
	char *decryptedText = reader.ReadToArray(strFileDecrypted.c_str(), decrLen);

	unsigned int plainLen;
	char *plaintext = reader.ReadToArray(strFileName.c_str(), plainLen);

	if (decrLen != plainLen)
		return false;

	return (std::strncmp((char*)decryptedText, (char*)plaintext, decrLen) == 0);
}

void DoTests()
{
	std::string strPathToTheKeystore = "../keystore1.jceks";
	std::string strKeystorePassword = "haslo1";
	std::string strKeyIdentifier = "aesklucz256";
	std::string strSecretKeyPassword = "haslo1";

	std::string strFileToEncrypt;// = "../test1.txt";
	std::string strFileEcrypted;// = "../test1Enc.txt";
	std::string strFileDecrypted;// = "../test1Dec.txt";

	std::vector<std::pair<const EVP_CIPHER*, std::string>> modes = { 
		std::make_pair(EVP_aes_256_cbc(), "CBC"),
		std::make_pair(EVP_aes_256_ctr(), "CTR"),
		std::make_pair(EVP_aes_256_ofb(), "OFB")
	};

	int maxTests = 3;
	std::string strTestFilePrefix = "../test";
	for (int i = 1; i <= 3; ++i)
	{
		strFileToEncrypt = strTestFilePrefix + std::to_string(i) + ".txt";
		strFileEcrypted = strTestFilePrefix + std::to_string(i) + "Enc.txt";
		strFileDecrypted = strTestFilePrefix + std::to_string(i) + "Dec.txt";
		for (auto modePair : modes)
		{
			if (SingleTest(strFileToEncrypt, strPathToTheKeystore, strKeystorePassword, strKeyIdentifier,
				strSecretKeyPassword, modePair.first, strFileEcrypted, strFileDecrypted))
				std::cout << "test " << i << " in " << modePair.second << " mode passed\n";
			else
				std::cout << "test " << i << " in " << modePair.second << " mode failed\n";
		}
	}
}

// ./program enc "file.txt" -m OFB/CTR/CBC -keystorepath keystorePath -keystorepass keystorepass -keyidentifier keyidentifier -keypass keypass
// usage: ./program enc file.txt CBC keystorePath keystorePass keyIdentifier keyPass
// usage example: EncDecCryptoList3.exe enc ../pliczek.txt CBC ../keystore1.jceks haslo1 aesklucz256 haslo1
// EncDecCryptoList3.exe dec ../pliczekEnc.txt CBC ../keystore1.jceks haslo1 aesklucz256 haslo1
// EncDecCryptoList3.exe TEST
void ProcessCommands(int argc, char **argv)
{
	//dumb processing
	if (argc != 8 && argc != 2)
	{
		std::cout << "Not correct number of parameters\n";
		std::cout << "example usage:\n";
		std::cout << "EncDecCryptoList3.exe enc file.txt CBC keystorePath keystorePass keyIdentifier keyPass\n";
		return;
	}

	if (argc == 2)
	{
		if (std::strcmp(argv[1], "TEST") == 0)
		{
			DoTests();
		}
		else
			return;
	}

	std::string strFileToEncDec = argv[2];// = "pliczek.txt";
	std::string strPathToTheKeystore = argv[4];// = "keystore1.jceks";
	std::string strKeystorePassword = argv[5];// = "haslo1";
	std::string strKeyIdentifier = argv[6];// = "aesklucz256";
	std::string strSecretKeyPassword = argv[7];// = "haslo1";

	/*std::cout << strFileToEncDec << std::endl;
	std::cout << strPathToTheKeystore << std::endl;
	std::cout << strKeystorePassword << std::endl;
	std::cout << strKeyIdentifier << std::endl;
	std::cout << strSecretKeyPassword << std::endl;*/

	const EVP_CIPHER* evp_cipher_mode;
	if (std::strcmp(argv[3], "CBC") == 0)
		evp_cipher_mode = EVP_aes_256_cbc();
	else if(std::strcmp(argv[3], "CTR") == 0)
		evp_cipher_mode = EVP_aes_256_ctr();
	else if (std::strcmp(argv[3], "OFB") == 0)
		evp_cipher_mode = EVP_aes_256_ofb();
	else
	{
		std::cout << "Wrong operation mode: " << argv[3] << std::endl;
		return;
	}

	AES256 aes;
	if (std::strcmp(argv[1], "enc") == 0)
	{
		aes.EncryptFile(strFileToEncDec, strPathToTheKeystore, strKeystorePassword, strKeyIdentifier, strSecretKeyPassword, evp_cipher_mode);
		std::cout << "encrypted file " << strFileToEncDec << std::endl;
	}
	else if (std::strcmp(argv[1], "dec") == 0)
	{
		aes.DecryptFile(strFileToEncDec, strPathToTheKeystore, strKeystorePassword, strKeyIdentifier, strSecretKeyPassword, evp_cipher_mode);
		std::cout << "decrypted file " << strFileToEncDec << std::endl;
	}
	else
		std::cout << "Wrong encrypt/decrypt mode: " << argv[1] << std::endl;
}

int main(int argc, char **argv)
{
	if (!InitLibraries())
		return -1;

	//ProcessCommands(argc, argv);
	//DoTests();

	//CkJavaKeyStore jceks;
	//bool success = jceks.LoadFile("haslo1", "keystore1.jceks");
	//if (success != true) {
	//	std::cout << jceks.lastErrorText() << "\r\n";
	//	return 1;
	//}

	//std::cout << "numSecretKeys: " << jceks.get_NumSecretKeys() << std::endl;
	//CkString ckstrPrivKey;
	////TODO FindSecretKey po keyIdentyfikatorze
	////CkPrivateKey *ck = jceks.FindPrivateKey("haslo1", "aesklucz256", true);

	//jceks.GetSecretKey("haslo1", 0, "base64", ckstrPrivKey);
	//if(!ckstrPrivKey.isEmpty())
	//	std::cout << ckstrPrivKey.getString() << std::endl;
	//
	//char *strPrivKey = const_cast<char *>(ckstrPrivKey.getString());

	//unsigned char *realiv = (unsigned char *)"45e85942862382751070390ae1bf11a6";
	//unsigned char *realKey= (unsigned char *)"328hsaf23234lsadf82392jbsajdfasddjsopsadfli239sdladfo2mnbfjelapq";
	//unsigned char *newplain = (unsigned char*)"For my purposes I decided to use the AES API directly. I made this decision based on the fact that I seemed to get further faster with the examples that used the AES API. As I gain proficiency with OpenSSL I'll be able to come back later and (hopefully) swallow the EVP API if ";
	//unsigned char *ciphertext = new unsigned char[2 * strlen((char*)newplain)];

	std::string strFileToEncrypt = "../pliczek.txt";
	std::string strPathToTheKeystore = "../keystore1.jceks";
	std::string strKeystorePassword = "haslo1";
	std::string strKeyIdentifier = "aesklucz256";
	std::string strSecretKeyPassword = "haslo1";

	AEScbcAttackOnModifiedIVGenerator aesAttack(strPathToTheKeystore, strKeystorePassword, strKeyIdentifier, strSecretKeyPassword);
	aesAttack.CPAAttack();

	//AES256 aes;
	//aes.EncryptFile(strFileToEncrypt, strPathToTheKeystore, strKeystorePassword, strKeyIdentifier, strSecretKeyPassword, EVP_aes_128_ofb());

	//std::string strFileToDecrypt = "../pliczekEnc.txt";
	//aes.EncryptFile(strFileToDecrypt, strPathToTheKeystore, strKeystorePassword, strKeyIdentifier, strSecretKeyPassword, EVP_aes_128_ofb());
	//aes.DecryptFile(strFileToDecrypt, strPathToTheKeystore, strKeystorePassword, strKeyIdentifier, strSecretKeyPassword, EVP_aes_128_ofb());

	//printf("Input: %s %d\n\n\n", newplain, strlen((char*)newplain));

	//int ciphertext_len = aes.Encrypt(newplain, strlen((char*)newplain), realKey, realiv, ciphertext, EVP_aes_256_ctr());
	////printf("Output: %s %d\n\n\n", ciphertext, ciphertext_len);

	///*Base64 base64;
	//char *base64EncodeOutput;
	//base64.Encode(ciphertext, ciphertext_len, &base64EncodeOutput);
	//printf("Output (base64): %s\n", base64EncodeOutput);

	//unsigned char *base64DecodedOutput;
	//int base64DecodedLen;
	//base64.Decode(base64EncodeOutput, &base64DecodedOutput, &base64DecodedLen);*/

	//Base64 base64;
	//std::string strCipher((char*)ciphertext);
	//std::string base64enc = base64.base64_encode(strCipher);

	//std::cout << base64enc << std::endl;

	//std::string strAgainCipher = base64.base64_decode(base64enc);
	//if (strCipher == strAgainCipher) std::cout << "udalo sie\n";

	//unsigned char *decodedPlaintext = new unsigned char[strlen((char*)newplain) + 1];
	//int plaintext_len = aes.Decrypt(ciphertext, ciphertext_len, realKey, realiv, decodedPlaintext, EVP_aes_256_ctr());
	//decodedPlaintext[strlen((char*)newplain)] = '\0';
	//printf("Output Decoded: %s %d\n", decodedPlaintext, strlen((char*)newplain));

	
	/* Clean up */
	EVP_cleanup();
	ERR_free_strings();
    return 0;
}

