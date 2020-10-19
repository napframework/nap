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

#ifdef _WIN32 
	#include <dll.h>
#endif
#include <rsa.h>

RTTI_BEGIN_CLASS(nap::LicenseConfiguration)
	RTTI_PROPERTY("License",	&nap::LicenseConfiguration::mLicense,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Key",		&nap::LicenseConfiguration::mKey,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LicenseService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	constexpr const char* LicenceToken = "LICENSE@";


	LicenseService::LicenseService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	LicenseService::~LicenseService()
	{ }


	bool LicenseService::validateLicense(const nap::PublicKey& publicKey, LicenseInformation& outInformation, utility::ErrorState& error)
	{
		// Ensure the user provided a license
		if (!error.check(hasLicense(), "No license provided"))
			return false;

		// Ensure license exists
		std::string lf = utility::stringFormat("%s/%s", getCore().getProjectInfo()->getProjectDir().c_str(), mLicense.c_str());
		if (!error.check(utility::fileExists(lf), "Can't find license: %s", lf.c_str()))
			return false;

		// Ensure the user provided a key
		if (!error.check(hasKey(), "No license key provided"))
			return false;

		// Ensure signature (key) exists
		std::string sf = utility::stringFormat("%s/%s", getCore().getProjectInfo()->getProjectDir().c_str(), mSignature.c_str());
		if (!error.check(utility::fileExists(lf), "Can't find license key: %s", sf.c_str()))
			return false;

		// Verify license using provided public application key
		if (!error.check(RSAVerifyFile(publicKey.getKey(), lf, sf), "Invalid license"))
			return false;

		// TODO: The RSAVerifyFile function already loads the license, but when using cryptopp (compiled with msvc 2015),
		// I am unable to first load the file to string and use that as a source for the verification operation -> runtime memory error
		std::string user_license;
		if (!utility::readFileToString(lf, user_license, error))
			return false;

		// Remove first part
		assert(utility::startsWith(user_license, LicenceToken, true));
		user_license.erase(0, strlen(LicenceToken));

		// Split using delimiter and create map of arguments
		std::vector<std::string> output = utility::splitString(user_license, '|');
		std::unordered_map<std::string, std::string> arguments;
		for (const auto& part : output)
		{
			std::vector<std::string> argument = utility::splitString(part, ':');
			if(argument.size() > 1)
				arguments.emplace(std::make_pair(argument[0], argument[1]));
		}

		// If an expiration date is specified check if it expired
		auto it = arguments.find("date");
		if (it != arguments.end())
		{
			SystemTimeStamp expiration_date;
			if (!getExpirationDate((*it).second, expiration_date, error))
			{
				error.fail("Unable to extract expiration date");
				return false;
			}

			// Check if it's expired
			DateTime current_time = getCurrentDateTime();
			if (!error.check(current_time.getTimeStamp() < expiration_date, "License expired"))
				return false;

			// Set date
			outInformation.mDate = Date(expiration_date);
		}

		// Populate other arguments
		setArgument(arguments, "mail", outInformation.mMail);
		setArgument(arguments, "name", outInformation.mName);
		setArgument(arguments, "application", outInformation.mApp);

		return true;
	}


	bool LicenseService::init(utility::ErrorState& error)
	{
		// Get configuration and check if a license is provided
		// Providing no license (at all) is allowed, validation will in that case always fail
		nap::LicenseConfiguration* license_config = getConfiguration<LicenseConfiguration>();
		mLicense = license_config->mLicense;
		mSignature = license_config->mKey;
		return true;
	}


	bool LicenseService::RSAVerifyFile(const std::string& publicKey, const std::string& licenseFile, const std::string& signatureFile)
	{
		try
		{
			// Load public key
			CryptoPP::StringSource pub_file(publicKey.c_str(), true, new CryptoPP::HexDecoder);
			CryptoPP::RSASS<CryptoPP::PKCS1v15, CryptoPP::SHA1>::Verifier pub(pub_file);

			// Load license signature file and ensure byte length matches
			CryptoPP::FileSource signature_file(signatureFile.c_str(), true, new CryptoPP::HexDecoder);
			if (signature_file.MaxRetrievable() != pub.SignatureLength())
				return false;

			// Copy into signature
			CryptoPP::SecByteBlock signature(pub.SignatureLength());
			signature_file.Get(signature, signature.size());

			// Load license and verify
			CryptoPP::SignatureVerificationFilter *verifier_filter = new CryptoPP::SignatureVerificationFilter(pub);
			verifier_filter->Put(signature, pub.SignatureLength());
			
			CryptoPP::FileSource license_file(licenseFile.c_str(), true, verifier_filter);
			return verifier_filter->GetLastResult();
		}
		catch (const std::exception& e)
		{
			nap::Logger::error(e.what());
			return false;
		}
	}


	void LicenseService::setArgument(const std::unordered_map<std::string, std::string>& args, const std::string& key, std::string& outValue)
	{
		auto it = args.find(key);
		outValue = it != args.end() ? (*it).second : std::string("not provided");
	}


	bool LicenseService::getExpirationDate(const std::string& date, SystemTimeStamp& outDate, utility::ErrorState& error)
	{
		std::vector<std::string> parts = utility::splitString(date, '/');
		if (!error.check(parts.size() == 3, "%s: Invalid date, format should be: '31/12/2025'", date.c_str()))
			return false;

		outDate = createTimestamp(std::stoi(parts[2]), std::stoi(parts[1]), std::stoi(parts[0]), 0, 0);
		return true;
	}

}