/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "commandline.h"

// Required for cryptopp library on windows
#ifdef _WIN32 
	#include <dll.h>
#endif
#include <rsa.h>
#include <hex.h>
#include <randpool.h>
#include <files.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

/**
 * Generates a public / private RSA key pair.
 * @param keyLength number of bits
 * @param privFilename private key filename
 * @param pubFilename public key filename
 * @param seed random seed
 */
bool GenerateRSAKey(unsigned int keyLength, const std::string& privFilename, const std::string& pubFilename, const std::string& seed)
{
	try
	{
        int				ret = 0;
        RSA				*r = NULL;
        BIGNUM			*bne = NULL;
        BIO				*bp_public = NULL, *bp_private = NULL;

        int				bits = 2048;
        unsigned long	e = RSA_F4;

        // 1. generate rsa key
        bne = BN_new();
        ret = BN_set_word(bne,e);
        if(ret != 1){
            goto free_all;
        }

        r = RSA_new();
        ret = RSA_generate_key_ex(r, bits, bne, NULL);
        if(ret != 1){
            goto free_all;
        }

        // 2. save public key
        bp_public = BIO_new_file(pubFilename.c_str(), "w+");
        ret = PEM_write_bio_RSAPublicKey(bp_public, r);
        if(ret != 1){
            goto free_all;
        }

        // 3. save private key
        bp_private = BIO_new_file(privFilename.c_str(), "w+");
        ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);

        // 4. free
        free_all:

        BIO_free_all(bp_public);
        BIO_free_all(bp_private);
        RSA_free(r);
        BN_free(bne);

        return (ret == 1);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl << "Unable to save keys" << std::endl;
		return false;
	}
	return true;
}


/**
 * Generates a unique public / private RSA key. 
 * Input arguments: 
 * -o	output directory, required and must exist
 * -s	key seed, for example the name of the app
 * -n	key name, defaults to 'key'
 * Returns 0 on success, -1 on failure
 *
 * Example:
 * ~~~~~
 * keygen.exe -n new_key -o c:/keys
 * ~~~~~
 */
int main(int argc, char* argv[])
{
	// Parse command-line
	CommandLine commandLine;
	if (!CommandLine::parse(argc, argv, commandLine))
		return -1;

	// public key
	std::ostringstream pub_loc;
	pub_loc << commandLine.mOutputDirectory << "/" << commandLine.mKeyName << ".public";

	// private key
	std::ostringstream pri_loc;
	pri_loc << commandLine.mOutputDirectory << "/" << commandLine.mKeyName << ".private";

	// Generate
	if (!GenerateRSAKey(1024, pri_loc.str(), pub_loc.str(), commandLine.mSeed))
		return -1;

	std::cout << "Successfully generated keys: " << std::endl;
	std::cout << "Public:    " << pub_loc.str()  << std::endl;
	std::cout << "Private:   " << pri_loc.str()  << std::endl;

	// All good
	return 0;
}
