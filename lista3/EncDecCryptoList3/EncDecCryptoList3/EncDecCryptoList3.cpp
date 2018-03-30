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

//#include <chilkat\include\CkJavaKeyStore.h>
//#include <chilkat\include\CkCert.h>
//#include <chilkat\include\CkPrivateKey.h>
//#include <chilkat\include\CkCertChain.h>
//#include <chilkat\include\C_CkCert.h>
//#include <chilkat\include\CkGlobal.h>
#include <string.h>
//#include <stdio.h>
#include <iostream>

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

int main()
{
	if (!InitLibraries())
		return -1;

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

	std::string strFileToEncrypt = "pliczek.txt";
	std::string strPathToTheKeystore = "keystore1.jceks";
	std::string strKeystorePassword = "haslo1";
	std::string strKeyIdentifier = "aesklucz256";
	std::string strSecretKeyPassword = "haslo1";

	AES256 aes;
	aes.EncryptFile(strFileToEncrypt, strPathToTheKeystore, strKeystorePassword, strKeyIdentifier, strSecretKeyPassword, EVP_aes_128_cbc());

	std::string strFileToDecrypt = "pliczekEnc.txt";
	aes.DecryptFile(strFileToDecrypt, strPathToTheKeystore, strKeystorePassword, strKeyIdentifier, strSecretKeyPassword, EVP_aes_128_cbc());

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

