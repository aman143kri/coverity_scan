#ifndef CIPHER_H
#define CIPHER_H

#include <string>
#include <openssl/evp.h>

class Cipher
{
    private:
        const int KEY_LENGTH;
	    const int BUFFER_SIZE;
		int aesInit(unsigned char * userKey, unsigned int keyLen, unsigned char * salt, EVP_CIPHER_CTX * ctx, bool encrypt);

	public:
        Cipher();
		std::string Decrypt(std::string ciphertext, std::string userKey, int * cipherLength);
		std::string Encrypt(std::string inputString, std::string userKey, int * plainLength);
};

#endif