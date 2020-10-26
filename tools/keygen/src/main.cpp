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
		// DEREncode() changed to Save() at Issue 569.
		CryptoPP::RandomPool randPool;
		randPool.IncorporateEntropy((unsigned char*)seed.data(), seed.size());

		CryptoPP::RSAES_OAEP_SHA_Decryptor priv(randPool, keyLength);
		CryptoPP::HexEncoder privFile(new CryptoPP::FileSink(privFilename.c_str()));
		priv.AccessMaterial().Save(privFile);
		privFile.MessageEnd();

		CryptoPP::RSAES_OAEP_SHA_Encryptor pub(priv);
		CryptoPP::HexEncoder pubFile(new CryptoPP::FileSink(pubFilename.c_str()));
		pub.AccessMaterial().Save(pubFile);
		pubFile.MessageEnd();
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