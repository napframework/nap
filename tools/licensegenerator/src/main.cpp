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

constexpr const char* licenceToken = "LICENSE@";

/**
 * Creates, signs and saves the license
 * @param privFilename path to private key, used to sign the license
 * @param license the license to sign and save
 * @param signatureFilename path to signature file
 * @param licenseFileName to license file
 */
static bool signLicense(const std::string& privFilename, const std::string& license, const std::string& signatureFilename, const std::string& licenseFileName)
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
 * Creates, signs and saves a license.
 * Use this tool to create a signed license that is compatible with mod_naplicense.
 *
 * Required arguments: 
 * -k	path to private key
 * -a	application name
 * -f	client first name
 * -l	client last name
 * -o	output directory
 *
 * Optional arguments:
 * -m	client mail address
 * -d	license expiry date
 * 
 * If the date is not specified the license is not bound to an end date.
 * Output format of human readable license = '.license'
 * Output format of signed license = '.key'
 * Returns 0 on success, -1 on failure
 *
 * Example:
 * ~~~~~
 * licensegenerator.exe -k c:/keys/key.private -f ben -l davis -a myapp -m ben@davis.com -d 30/12/2025 -o c:/license
 * ~~~~~
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
		<< commandLine.mApplication << "_" << commandLine.mFistName << "_" << commandLine.mLastName << 
		".key";

	// License output file
	std::ostringstream lic_loc;
	lic_loc << commandLine.mOutputDirectory << "/" 
		<< commandLine.mApplication << "_" << commandLine.mFistName << "_" << commandLine.mLastName << 
		".license";

	// Create license content
	std::ostringstream lic_content;
	lic_content << LicenceToken <<
		"application:" << commandLine.mApplication << "|" <<
		"name:" << commandLine.mFistName << " " << commandLine.mLastName << "|" <<
		"mail:" << commandLine.mMail << "|" <<
		"date:" << commandLine.mDate;

	// Create license
	if (!signLicense(commandLine.mKey, lic_content.str(), key_loc.str(), lic_loc.str()))
		return -1;

	std::cout << "Successfully created and signed license" << std::endl;
	std::cout << "Key location:         " << key_loc.str() << std::endl;
	std::cout << "License location:     " << lic_loc.str() << std::endl;

	// All good
	return 0;
} 