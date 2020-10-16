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
#include <validate.h>

using namespace CryptoPP;
using namespace std;


bool signLicense(const std::string& privFilename, const std::string& license, const std::string& signatureFilename, const std::string& licenseFileName)
{
	try
	{
		// Write signed license version
		FileSource privFile(privFilename.c_str(), true, new HexDecoder);
		RSASS<PKCS1v15, SHA1>::Signer priv(privFile);
		NonblockingRng rng;
		StringSource f(license.c_str(), true, new SignerFilter(rng, priv, new HexEncoder(new FileSink(signatureFilename.c_str()))));

		// Write regular text version
		FileSink l(licenseFileName.c_str());
		l.Put((const unsigned char*)license.data(), license.size());
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl << "Unable to create license" << std::endl;
		return false;
	}
	return true;
}


/**
 * Creates a signed license.
 */
int main(int argc, char* argv[])
{
	// Parse command-line
	CommandLine commandLine;
	if (!CommandLine::parse(argc, argv, commandLine))
		return -1;

	// Signed license output file
	std::ostringstream key_loc;
	key_loc << commandLine.mOutputDirectory << "/" 
		<< commandLine.mSignature << "_" << commandLine.mFistName << "_" << commandLine.mLastName << 
		"_key.txt";

	// License output file
	std::ostringstream lic_loc;
	lic_loc << commandLine.mOutputDirectory << "/" 
		<< commandLine.mSignature << "_" << commandLine.mFistName << "_" << commandLine.mLastName << 
		"_license.txt";

	// Create license content
	std::ostringstream lic_content;
	lic_content << "LICENSE@" <<
		commandLine.mSignature << ":" <<
		commandLine.mFistName << "_" << commandLine.mLastName << ":" <<
		commandLine.mMail << ":" <<
		commandLine.mDate;

	// Create license
	if (!signLicense(commandLine.mKey, lic_content.str(), key_loc.str(), lic_loc.str()))
		return -1;

	std::cout << "Successfully created and signed license" << std::endl;
	std::cout << "Key location:         " << key_loc.str() << std::endl;
	std::cout << "License location:     " << lic_loc.str() << std::endl;

	// All good
	return 0;
} 