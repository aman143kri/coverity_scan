/*	cipher.cpp
*
*	Source code organizational unit for all functions that deals with cipher.
*
*/

#include <algorithm>
#include <vector>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include "../headers/modules.h"
#include "../headers/cipher.h"

using namespace std;

Cipher :: Cipher() 
	: KEY_LENGTH(256), BUFFER_SIZE(2048) 
{}

int Cipher :: aesInit(unsigned char * userKey, unsigned int keyLen, unsigned char * salt, EVP_CIPHER_CTX * ctx, bool encrypt) {
	int keySize;
	int rounds = 10;
	unsigned char key[32];
	unsigned char iv[32];

	keySize = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt, userKey, keyLen, rounds, key, iv);

	if(encrypt) {
		EVP_CIPHER_CTX_init(ctx);
		EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
	} else {
		EVP_CIPHER_CTX_init(ctx);
		EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
	}

	return 0;
}

std::string Cipher :: Decrypt(std::string ciphertext, std::string userKey, int * cipherLength) {
	int plainLength;

	unsigned char * plaintext = new unsigned char[*cipherLength];
	EVP_CIPHER_CTX * dec = EVP_CIPHER_CTX_new();

	int finalLength = 0;
	int keyLength = userKey.length();
	int paddingNeeded = (KEY_LENGTH - userKey.length() % KEY_LENGTH);
	std::vector<unsigned char> paddedKeyVec(userKey.begin(), userKey.end());
	for(int i=0; i<paddingNeeded; i++) {
		paddedKeyVec.push_back(0);
	}
	unsigned char * paddedKey = &paddedKeyVec[0];

	aesInit(paddedKey, KEY_LENGTH, 0, dec, false);

	std::vector<unsigned char> ciphertextChar(ciphertext.begin(), ciphertext.end());

	EVP_DecryptUpdate(dec, plaintext, &plainLength, &ciphertextChar[0], ciphertext.length());
	*cipherLength += plainLength;

	EVP_DecryptFinal_ex(dec, plaintext + plainLength, &plainLength);
	*cipherLength += plainLength;

	std::string plaintextStr(reinterpret_cast<char*>(plaintext));
	return plaintextStr;
}

std::string Cipher :: Encrypt(std::string inputString, std::string userKey, int * plainLength) {
	EVP_CIPHER_CTX * enc = EVP_CIPHER_CTX_new();

	int inputLength = inputString.length();

	int paddingNeeded = (AES_BLOCK_SIZE - inputLength % AES_BLOCK_SIZE);
	std::vector<unsigned char> paddedInput(inputString.begin(), inputString.end());
	for(int i=0; i<paddingNeeded; i++) {
		paddedInput.push_back(0);
	}
	inputLength += paddingNeeded;

	unsigned char * paddedString = &paddedInput[0];

	paddingNeeded = (KEY_LENGTH - userKey.length() % KEY_LENGTH);
	std::vector<unsigned char> paddedKeyVec(userKey.begin(), userKey.end());
	for(int i=0; i<paddingNeeded; i++) {
		paddedKeyVec.push_back(0);
	}
	unsigned char * paddedKey = &paddedKeyVec[0];

	aesInit(paddedKey, KEY_LENGTH, 0, enc, true);
	
	inputLength = paddedInput.size();
	int ciphertextLength = paddedInput.size() + AES_BLOCK_SIZE;
	unsigned char * ciphertext = new unsigned char[ciphertextLength];
	
	EVP_EncryptUpdate(enc, ciphertext, &ciphertextLength, paddedString, inputLength);

	EVP_EncryptFinal_ex(enc, ciphertext + ciphertextLength, &inputLength);

	*plainLength = ciphertextLength + inputLength;
	std::string ciphertextStr(reinterpret_cast<char*>(ciphertext));

	return ciphertextStr;
}