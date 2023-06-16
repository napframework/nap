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
#include <osrng.h>
#include <files.h>
#include <hex.h>
#include <cctype>
#include <chrono>

using namespace CryptoPP;
using namespace std;
using SystemClock = std::chrono::system_clock;
using SystemTimeStamp = std::chrono::time_point<SystemClock>;

constexpr const char* licenceToken = "LICENSE@";
constexpr const char* licenseExtension = "license";
constexpr const char* keyExtension = "key";
constexpr const char* defaultSigningSceme = "RSASS_PKCS1v15_SHA1";
constexpr const char* supportedSigningSchemes[] =
{
	defaultSigningSceme,
	"RSASS_PKCS1v15_SHA224",
	"RSASS_PKCS1v15_SHA256",
	"RSASS_PKCS1v15_SHA384",
	"RSASS_PKCS1v15_SHA512",
	""
};

/**
 * Returns if the given date exists
 * @param m month
 * @param d day
 * @param y year
 */
static bool dateExists(int m, int d, int y)
{
	//Gregorian dates started in 1582
	if (!(1582 <= y))
		return false;
	if (!(1 <= m && m <= 12))
		return false;
	if (!(1 <= d && d <= 31))
		return false;
	if ((d == 31) && (m == 2 || m == 4 || m == 6 || m == 9 || m == 11))
		return false;
	if ((d == 30) && (m == 2))
		return false;
	if ((m == 2) && (d == 29) && (y % 4 != 0))
		return false;
	if ((m == 2) && (d == 29) && (y % 400 == 0))
		return true;
	if ((m == 2) && (d == 29) && (y % 100 == 0))
		return false;
	if ((m == 2) && (d == 29) && (y % 4 == 0))
		return true;

	return true;
}


/**
 * Checks if the given string is a number (positive only)
 * @return if the string is a number
 * @param s the string to check
 */
static bool isNumber(const std::string& s)
{
	return !s.empty() && std::find_if(s.begin(),
		s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}


/**
 * Helper function to instantiate the correct signing class with private key
 * @return the instantiated Signer class
 * @param privFilename path to private key, used to sign the license
 * @param signingScheme the signing scheme
 */
static std::unique_ptr<PK_Signer> createSigner(const std::string& privFilename, const std::string& signingScheme)
{
	FileSource privFile(privFilename.c_str(), true, new HexDecoder);
	std::unique_ptr<PK_Signer> signer;

	if (signingScheme == "RSASS_PKCS1v15_SHA1")
		signer.reset(new RSASS<PKCS1v15, SHA1>::Signer(privFile));
	else if (signingScheme == "RSASS_PKCS1v15_SHA224")
		signer.reset(new RSASS<PKCS1v15, SHA224>::Signer(privFile));
	else if (signingScheme == "RSASS_PKCS1v15_SHA256")
		signer.reset(new RSASS<PKCS1v15, SHA256>::Signer(privFile));
	else if (signingScheme == "RSASS_PKCS1v15_SHA384")
		signer.reset(new RSASS<PKCS1v15, SHA384>::Signer(privFile));
	else if (signingScheme == "RSASS_PKCS1v15_SHA512")
		signer.reset(new RSASS<PKCS1v15, SHA512>::Signer(privFile));

	return signer;
}


/**
 * Creates, signs and saves the license
 * @param privFilename path to private key, used to sign the license
 * @param signingScheme the signing scheme
 * @param license the license to sign and save
 * @param signatureFilename path to signature file
 * @param licenseFileName to license file
 */
static bool signLicense(const std::string& privFilename, const std::string& signingScheme, const std::string& license, const std::string& signatureFilename, const std::string& licenseFileName)
{
	try
	{
		std::unique_ptr<PK_Signer> signer = createSigner(privFilename, signingScheme);
		if (!signer)
		{
			// Someone forgot to update createSigner(). This is undesirable.
			std::cerr << "Signing scheme not fully implemented: " << signingScheme << std::endl << "Unable to create license" << std::endl;
			return false;
		}

		// Write signed license version
		NonblockingRng rng;
		StringSource f(license.c_str(), true, new SignerFilter(rng, *signer, new HexEncoder(new FileSink(signatureFilename.c_str()))));

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
 * Ensures the given date as string is valid
 * @param date date as string
 * @return if the date is correct
 */
static bool validateDate(const std::string& date)
{
	std::vector<int> parts;
	std::stringstream ss(date);
	std::string item;

	// Split and ensure every entry is a number
	while (std::getline(ss, item, '/'))
	{
		if (!isNumber(item))
		{
			std::cout << "Unable to extract date, contains invalid characters: " << item << std::endl;
			std::cout << "Format: day/month/year -> 20/12/2025" << std::endl;
			return false;
		}
		parts.emplace_back(std::stoi(item));
	}

	// Ensure there are 3 parts (day, month, year)
	if (parts.size() != 3)
	{
		std::cout << "Unable to extract date, unsupported format: " << date << std::endl;
		std::cout << "Format: day/month/year -> 20/12/2025" << std::endl;
		return false;
	}

	// Make sure it's a valid date
	if (!dateExists(parts[1], parts[0], parts[2]))
	{
		std::cout << "Invalid date: " << date << std::endl;
		std::cout << "Format: day/month/year -> 20/12/2025" << std::endl;
		return false;
	}
	return true;
}


/**
 * Ensures the given signing scheme as string is valid
 * @param signingScheme signing scheme as string
 * @return if the signing scheme is correct
 */
static bool validateSigningScheme(const std::string& signingScheme)
{
	// Validate the signing scheme is supported
	for (int i = 0; supportedSigningSchemes[i][0] != '\0'; ++i)
	{
		if (signingScheme == supportedSigningSchemes[i])
		{
			return true;
		}
	}

	std::cout << "Invalid signing scheme: " << signingScheme << std::endl;
	std::cout << "Supported signing schemes: " << std::endl;
	for (int i = 0; supportedSigningSchemes[i][0] != '\0'; ++i)
	{
		std::cout << "  " << supportedSigningSchemes[i] << std::endl;
	}
	return false;
}


/**
 * Creates, signs and saves a license.
 * Use this tool to create a signed license that is compatible with naplicense.
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
 * -t	additional message (tag)
 * -s	signing scheme
 * -i	unique machine id
 * 
 * Without a date the license is valid indefinitely.
 * The uuid ties the license to a specific machine. No uuid creates a 'floating' license.
 * For backwards compatibility with older versions of naplicense, do not specify the signing scheme.
 * Output format of human readable license = '.license'
 * Output format of signed license = '.key'
 * Returns 0 on success, -1 on failure
 *
 * Example:
 * ~~~~~
 * licensegenerator -k c:/keys/key.private -s RSASS_PKCS1v15_SHA1 -f ben -l davis -a myapp -m ben@davis.com -d 30/12/2025 -t educational -o c:/license
 * ~~~~~
 */
int main(int argc, char* argv[])
{
	// Parse command-line
	CommandLine commandLine;
	if (!CommandLine::parse(argc, argv, commandLine))
		return -1;

	// Set signing scheme
	std::string signingScheme = defaultSigningSceme;
	if (!commandLine.mSignScheme.empty())
	{
		signingScheme = commandLine.mSignScheme;
		if (!validateSigningScheme(signingScheme))
			return -1;
	}

	// Create license content
	std::ostringstream lic_content;
	lic_content << licenceToken <<
		"application:" << commandLine.mApplication <<
		"|name:" << commandLine.mFistName << " " << commandLine.mLastName;

	// Add mail if provided
	if (!commandLine.mMail.empty())
		lic_content << "|mail:" << commandLine.mMail;

	// Add date if provided
	if (!commandLine.mDate.empty())
	{
		if (!validateDate(commandLine.mDate))
			return -1;
		lic_content << "|date:" << commandLine.mDate;
	}

	// Add tag (additional information) if provided
	if (!commandLine.mTag.empty())
		lic_content << "|tag:" << commandLine.mTag;

	// Add uuid (optional)
	if (!commandLine.mID.empty())
		lic_content << "|id:" << commandLine.mID;

	// Add issue time -> minutes since epoch
	SystemTimeStamp ctime = SystemClock::now();
	auto minutes = std::chrono::time_point_cast<std::chrono::minutes>(ctime);
	lic_content << "|issued:" << minutes.time_since_epoch().count();

	// Signed license output file
	std::ostringstream key_loc;
	key_loc << commandLine.mOutputDirectory << "/"
		<< commandLine.mApplication << "_" << commandLine.mFistName << "_" << commandLine.mLastName <<
		"." << keyExtension;

	// License output file
	std::ostringstream lic_loc;
	lic_loc << commandLine.mOutputDirectory << "/"
		<< commandLine.mApplication << "_" << commandLine.mFistName << "_" << commandLine.mLastName <<
		"." << licenseExtension;

	// Create license
	if (!signLicense(commandLine.mKey, signingScheme, lic_content.str(), key_loc.str(), lic_loc.str()))
		return -1;

	std::cout << "Successfully created and signed license" << std::endl;
	std::cout << "Key location:         " << key_loc.str() << std::endl;
	std::cout << "License location:     " << lic_loc.str() << std::endl;

	// All good
	return 0;
} 
