/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "commandline.h"

// Required for cryptopp library on windows
#ifdef _WIN32 
	#include <dll.h>
#endif

#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <filesystem>

/* A property query used for selecting algorithm implementations. */
static const char *propq = NULL;

/*
 * Generates an RSA public-private key pair and returns it.
 * The number of bits is specified by the bits argument.
 *
 * This uses the long way of generating an RSA key.
 */
static EVP_PKEY *generate_rsa_key_long(OSSL_LIB_CTX *libctx, unsigned int bits)
{
    EVP_PKEY_CTX *genctx = NULL;
    EVP_PKEY *pkey = NULL;
    unsigned int primes = 2;

    /* Create context using RSA algorithm. "RSA-PSS" could also be used here. */
    genctx = EVP_PKEY_CTX_new_from_name(libctx, "RSA", propq);
    if (genctx == NULL) {
        fprintf(stderr, "EVP_PKEY_CTX_new_from_name() failed\n");
        goto cleanup;
    }

    /* Initialize context for key generation purposes. */
    if (EVP_PKEY_keygen_init(genctx) <= 0) {
        fprintf(stderr, "EVP_PKEY_keygen_init() failed\n");
        goto cleanup;
    }

    /*
     * Here we set the number of bits to use in the RSA key.
     * See comment at top of file for information on appropriate values.
     */
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(genctx, bits) <= 0) {
        fprintf(stderr, "EVP_PKEY_CTX_set_rsa_keygen_bits() failed\n");
        goto cleanup;
    }

    /*
     * It is possible to create an RSA key using more than two primes.
     * Do not do this unless you know why you need this.
     * You ordinarily do not need to specify this, as the default is two.
     *
     * Both of these parameters can also be set via EVP_PKEY_CTX_set_params, but
     * these functions provide a more concise way to do so.
     */
    if (EVP_PKEY_CTX_set_rsa_keygen_primes(genctx, primes) <= 0) {
        fprintf(stderr, "EVP_PKEY_CTX_set_rsa_keygen_primes() failed\n");
        goto cleanup;
    }

    /*
     * Generating an RSA key with a number of bits large enough to be secure for
     * modern applications can take a fairly substantial amount of time (e.g.
     * one second). If you require fast key generation, consider using an EC key
     * instead.
     *
     * If you require progress information during the key generation process,
     * you can set a progress callback using EVP_PKEY_set_cb; see the example in
     * EVP_PKEY_generate(3).
     */
    fprintf(stdout, "Generating RSA key, this may take some time...\n");
    if (EVP_PKEY_generate(genctx, &pkey) <= 0) {
        fprintf(stderr, "EVP_PKEY_generate() failed\n");
        goto cleanup;
    }

    /* pkey is now set to an object representing the generated key pair. */

    cleanup:
    EVP_PKEY_CTX_free(genctx);
    return pkey;
}


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
        OSSL_LIB_CTX *libctx = NULL;
        auto key_pair = generate_rsa_key_long(libctx, keyLength);

        /* Output a PEM encoding of the public key. */
        auto* pub_key_file = fopen(pubFilename.c_str(), "w");
        if (PEM_write_PUBKEY(pub_key_file, key_pair) == 0)
        {
            fprintf(stderr, "Failed to output PEM-encoded public key\n");
        }

        /*
         * Output a PEM encoding of the private key. Please note that this output is
         * not encrypted. You may wish to use the arguments to specify encryption of
         * the key if you are storing it on disk. See PEM_write_PrivateKey(3).
         */
        auto* priv_key_file = fopen(privFilename.c_str(), "w");
        if (PEM_write_PrivateKey(priv_key_file, key_pair, NULL, NULL, 0, NULL, NULL) == 0)
        {
            fprintf(stderr, "Failed to output PEM-encoded private key\n");
        }
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
	if (!GenerateRSAKey(2048, pri_loc.str(), pub_loc.str(), commandLine.mSeed))
		return -1;

	std::cout << "Successfully generated keys: " << std::endl;
	std::cout << "Public:    " << pub_loc.str()  << std::endl;
	std::cout << "Private:   " << pri_loc.str()  << std::endl;

	// All good
	return 0;
} 