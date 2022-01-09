/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "licenseservice.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <unordered_map>
#include <nap/assert.h>

#ifdef _WIN32 
	#include <dll.h>
#endif
#include <rsa.h>
#include <hex.h>
#include <files.h>
#include <sha.h>

RTTI_BEGIN_CLASS(nap::LicenseConfiguration)
	RTTI_PROPERTY("LicenseDirectory",	&nap::LicenseConfiguration::mDirectory,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LicenseService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	inline constexpr const char* licenseToken = "LICENSE@";
	inline constexpr const char* licenseExtension = "license";
	inline constexpr const char* keyExtension = "key";

	static bool findFile(const char* extension, const std::vector<std::string>& files, std::string& outFile)
	{
		auto it = std::find_if(files.begin(), files.end(), [&](const auto& it)
		{
			return utility::getFileExtension(it) == extension;
		});

		bool file_found = it != files.end();
		outFile = file_found ? *it : "";
		return file_found;
	}


	static CryptoPP::PK_Verifier* getVerifier(const std::string& publicKey, nap::SigningScheme signingScheme)
	{
		CryptoPP::StringSource pub_file(publicKey.c_str(), true, new CryptoPP::HexDecoder);
		CryptoPP::PK_Verifier* verifier = nullptr;
		if (signingScheme == nap::SigningScheme::RSASS_PKCS1v15_SHA1)
			verifier = new CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA1>::Verifier(pub_file);
		else if (signingScheme == nap::SigningScheme::RSASS_PKCS1v15_SHA224)
			verifier = new CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA224>::Verifier(pub_file);
		else if (signingScheme == nap::SigningScheme::RSASS_PKCS1v15_SHA256)
			verifier = new CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA256>::Verifier(pub_file);
		else if (signingScheme == nap::SigningScheme::RSASS_PKCS1v15_SHA384)
			verifier = new CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA384>::Verifier(pub_file);
		else if (signingScheme == nap::SigningScheme::RSASS_PKCS1v15_SHA512)
			verifier = new CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA512>::Verifier(pub_file);

		return verifier;
	}


	static bool rsaVerifyFile(const std::string& publicKey, nap::SigningScheme signingScheme, const std::string& licenseFile, const std::string& signatureFile)
	{
		try
		{
			// Load public key
			std::unique_ptr<CryptoPP::PK_Verifier> verifier = std::unique_ptr<CryptoPP::PK_Verifier>(getVerifier(publicKey, signingScheme));

			// Load license signature file and ensure byte length matches
			CryptoPP::FileSource signature_file(signatureFile.c_str(), true, new CryptoPP::HexDecoder);
			if (signature_file.MaxRetrievable() != verifier->SignatureLength())
				return false;

			// Copy into signature
			CryptoPP::SecByteBlock signature(verifier->SignatureLength());
			signature_file.Get(signature, signature.size());

			// Load license and verify
			CryptoPP::SignatureVerificationFilter *verifier_filter = new CryptoPP::SignatureVerificationFilter(*verifier);
			verifier_filter->Put(signature, verifier->SignatureLength());

			CryptoPP::FileSource license_file(licenseFile.c_str(), true, verifier_filter);
			return verifier_filter->GetLastResult();
		}
		catch (const std::exception& e)
		{
			nap::Logger::error(e.what());
			return false;
		}
	}


	static void setArgument(const std::unordered_map<std::string, std::string>& args, const std::string& key, std::string& outValue)
	{
		auto it = args.find(key);
		outValue = it != args.end() ? (*it).second : std::string("not provided");
	}


	static bool getExpirationDate(const std::string& date, SystemTimeStamp& outDate)
	{
		std::vector<std::string> parts = utility::splitString(date, '/');
		NAP_ASSERT_MSG(parts.size() == 3, "invalid date format");
		outDate = createTimestamp(std::stoi(parts[2]), std::stoi(parts[1]), std::stoi(parts[0]), 0, 0);
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// LicenseService
	//////////////////////////////////////////////////////////////////////////

	LicenseService::LicenseService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	bool LicenseService::validateLicense(const nap::PublicKey& publicKey, LicenseInformation& outInformation, utility::ErrorState& error)
	{
		return validateLicense(publicKey.getKey(), nap::SigningScheme::RSASS_PKCS1v15_SHA1, outInformation, error);
	}


	bool LicenseService::validateLicense(const std::string& publicKey, LicenseInformation& outInformation, utility::ErrorState& error)
	{
		return validateLicense(publicKey, nap::SigningScheme::RSASS_PKCS1v15_SHA1, outInformation, error);
	}


	bool LicenseService::validateLicense(const nap::PublicKey& publicKey, nap::SigningScheme signingScheme, LicenseInformation& outInformation, utility::ErrorState& error)
	{
		return validateLicense(publicKey.getKey(), signingScheme, outInformation, error);
	}


	bool LicenseService::validateLicense(const std::string& publicKey, nap::SigningScheme signingScheme, LicenseInformation& outInformation, utility::ErrorState& error)
	{
		// Ensure the user provided a license
		if (!error.check(hasLicense(), "No .%s file found in: %s", licenseExtension, mDirectory.c_str()))
			return false;
		assert(utility::fileExists(mLicense));

		// Ensure the user provided a key
		if (!error.check(hasKey(), "No .%s file found in: %s", keyExtension, mDirectory.c_str()))
			return false;
		assert(utility::fileExists(mSignature));

		// Verify license using provided public application key
		if (!error.check(rsaVerifyFile(publicKey, signingScheme, mLicense, mSignature), "Invalid license"))
			return false;

		// TODO: The RSAVerifyFile function already loads the license, but when using cryptopp (compiled with msvc 2015),
		// I am unable to first load the file to string and use that as a source for the verification operation -> runtime memory error
		std::string user_license;
		if (!utility::readFileToString(mLicense, user_license, error))
			return false;

		// Remove first part
		assert(utility::startsWith(user_license, licenseToken, true));
		user_license.erase(0, strlen(licenseToken));

		// Split using delimiter and create map of arguments
		std::vector<std::string> output = utility::splitString(user_license, '|');
		std::unordered_map<std::string, std::string> arguments;
		for (const auto& part : output)
		{
			std::vector<std::string> argument = utility::splitString(part, ':');
			if (argument.size() > 1)
				arguments.emplace(std::make_pair(argument[0], argument[1]));
		}

		// Get issue date (minutes since epoch)
		auto issue_it = arguments.find("issued");
		if (!error.check(issue_it != arguments.end(), "License has no issue date"))
			return false;

		// Convert to system time
		std::chrono::minutes dur(std::stoi((*issue_it).second));
		SystemTimeStamp stamp_issued(dur);

		// If the current system time is less than the license issue time,
		// someone tried to reverse the clock or clock is not up to date
		if (!error.check(getCurrentTime() >= stamp_issued, "Invalid system clock"))
			return false;

		// Populate standards arguments
		setArgument(arguments, "mail", outInformation.mMail);
		setArgument(arguments, "name", outInformation.mName);
		setArgument(arguments, "application", outInformation.mApp);
		setArgument(arguments, "tag", outInformation.mTag);
		setArgument(arguments, "uuid", outInformation.mUuid);

		// If an expiration date is specified check if it expired
		outInformation.mExpires = false;
		auto it = arguments.find("date");
		if (it != arguments.end())
		{
			SystemTimeStamp expiration_date;
			if (!error.check(getExpirationDate((*it).second, expiration_date),
				"Unable to extract license expiration date"))
				return false;

			// Check if it's expired
			outInformation.mExpires = true;
			outInformation.mTime.setTimeStamp(expiration_date);
			if (!error.check(!outInformation.expired(), "License expired"))
				return false;
		}
		return true;
	}

	bool LicenseService::init(utility::ErrorState& error)
	{
		// Providing no license (at all) is allowed, validation will in that case always fail
		nap::LicenseConfiguration* license_config = getConfiguration<LicenseConfiguration>();
		
		// Patch license directory
		mDirectory = license_config->mDirectory;
		getCore().getProjectInfo()->patchPath(mDirectory);

		// ensure it exists
		if (!utility::dirExists(mDirectory))
		{
			nap::Logger::warn("License directory does not exist: %s", mDirectory.c_str());
			return true;
		}

		// Get all the files in that directory
		std::vector<std::string> license_files;
		utility::listDir(mDirectory.c_str(), license_files, true);

		// Find .license file
		if (!findFile(licenseExtension, license_files, mLicense))
		{
			nap::Logger::warn("Unable to find: .%s file in: %s", licenseExtension, mDirectory.c_str());
			return true;
		}

		// Find .key file
		if (!findFile(keyExtension, license_files, mSignature))
		{
			nap::Logger::warn("Unable to find: .%s file in: %s", keyExtension, mDirectory.c_str());
			return true;
		}
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// LicenseInformation
	//////////////////////////////////////////////////////////////////////////

	bool LicenseInformation::expired()
	{
		return this->canExpire() ? getCurrentTime() > mTime.getTimeStamp() : false;
	}
}
