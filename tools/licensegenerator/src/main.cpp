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
 * -t	additional message (tag)
 * 
 * If the date is not specified the license is not bound to an end date.
 * Output format of human readable license = '.license'
 * Output format of signed license = '.key'
 * Returns 0 on success, -1 on failure
 *
 * Example:
 * ~~~~~
 * licensegenerator -k c:/keys/key.private -f ben -l davis -a myapp -m ben@davis.com -d -t educational 30/12/2025 -o c:/license
 * ~~~~~
 */
int main(int argc, char* argv[])
{
	// Parse command-line
	CommandLine commandLine;
	if (!CommandLine::parse(argc, argv, commandLine))
		return -1;

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
	if (!commandLine.mUuid.empty())
		lic_content << "|uuid:" << commandLine.mUuid;

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
	if (!signLicense(commandLine.mKey, lic_content.str(), key_loc.str(), lic_loc.str()))
		return -1;

	std::cout << "Successfully created and signed license" << std::endl;
	std::cout << "Key location:         " << key_loc.str() << std::endl;
	std::cout << "License location:     " << lic_loc.str() << std::endl;

	// All good
	return 0;
} 
