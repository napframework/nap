/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <fstream>
#include "commandline.h"

// Required for cryptopp library on windows
#ifdef _WIN32 
	#include <dll.h>
#endif

#include "opensslapi.h"

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
    std::string private_key;
    std::string public_key;
	if (!nap::opensslapi::generateRSAKey(4096, private_key, public_key))
    {
        std::cout << "Failed to generate keys" << std::endl;
        return -1;
    }

    // Save public key
    std::ofstream pub_file(pub_loc.str());
    pub_file << public_key;
    pub_file.close();

    // Save private key
    std::ofstream priv_file(pri_loc.str());
    priv_file << private_key;
    priv_file.close();


	std::cout << "Successfully generated keys: " << std::endl;
	std::cout << "Public:    " << pub_loc.str()  << std::endl;
	std::cout << "Private:   " << pri_loc.str()  << std::endl;

	// All good
	return 0;
}
