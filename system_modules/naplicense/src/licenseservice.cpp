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
	#include <windows.h>
	#include <intrin.h>       
	#include <iphlpapi.h>
#elif  __linux__
    #include <ifaddrs.h>
    #include <netpacket/packet.h>
    #include <net/if.h>
#endif

// napopenssl includes
#include <opensslutils.h>

RTTI_BEGIN_CLASS(nap::LicenseConfiguration)
	RTTI_PROPERTY("LicenseDirectory",	&nap::LicenseConfiguration::mDirectory,		nap::rtti::EPropertyMetaData::Default, "License search directory")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LicenseService, "Validates a license using a public key")
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

bool digest_message(unsigned char *message, unsigned char **digest, unsigned int *digest_len);

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


	static void setArgument(const std::unordered_map<std::string, std::string>& args, const std::string& key, std::string& outValue)
	{
		auto it = args.find(key);
		outValue = it != args.end() ? (*it).second : "";
	}


	static bool getExpirationDate(const std::string& date, SystemTimeStamp& outDate)
	{
		std::vector<std::string> parts = utility::splitString(date, '/');
		NAP_ASSERT_MSG(parts.size() == 3, "invalid date format");
		outDate = createTimestamp(std::stoi(parts[2]), std::stoi(parts[1]), std::stoi(parts[0]), 0, 0);
		return true;
	}


#ifdef __linux__
	static bool generateMachineID(std::string& outID, nap::utility::ErrorState& error)
	{
		uint64 num_id = 0;
		struct ifaddrs* if_addresses = nullptr;
		if (!error.check(getifaddrs(&if_addresses) > -1, "Unable to access network interfaces"))
			return false;

		struct ifaddrs* interface = nullptr;
		for (interface = if_addresses; interface != nullptr; interface = interface->ifa_next)
		{
            // Skip loop back devices
            if(interface->ifa_flags & IFF_LOOPBACK)
                continue;

            // Update ID
			if ((interface->ifa_addr) && (interface->ifa_addr->sa_family == AF_PACKET))
			{
				sockaddr_ll* s = reinterpret_cast<sockaddr_ll*>(interface->ifa_addr);
				for (auto i = 0; i < s->sll_halen; i++)
                {
                    num_id += static_cast<uint64>(s->sll_addr[i]) << (i * 8);
                }
			}
		}
		freeifaddrs(if_addresses);

		// Add machine ID
		std::string id_str;
		if (!nap::utility::readFileToString("/etc/machine-id", id_str, error))
		{
			error.fail("Unable to read machine identifier");
			return false;
		}

        // hash and encode
        std::string hashed_id = utility::sha256(id_str);
		std::string id_encoded = utility::encode64(hashed_id);
        assert(id_encoded.size() >= 10);

        // convert to hex and truncate for readability
        std::stringstream ss;
        for(int i=0; i<10; ++i)
            ss << std::uppercase << std::hex << (int)id_encoded[i];
        outID = ss.str();

        return true;
	}

#elif _WIN32
	static bool generateMachineID(std::string& outID, nap::utility::ErrorState& error)
	{
		uint64 num_id = 0;

		//////////////////////////////////////////////////////////////////////////
		// Network adapters
		//////////////////////////////////////////////////////////////////////////

		// Make an initial call to GetAdaptersInfo to get the necessary size into the ulOutBufLen variable
		PIP_ADAPTER_INFO p_adapter_info = nullptr; ULONG buf_length = 0;
		if (GetAdaptersInfo(p_adapter_info, &buf_length) == ERROR_BUFFER_OVERFLOW)
		{
			p_adapter_info = (IP_ADAPTER_INFO*)std::malloc(buf_length);
			if (!error.check(p_adapter_info != nullptr, "Error allocating network information"))
				return false;
		}

		// Get adapter information
		if (!error.check(GetAdaptersInfo(p_adapter_info, &buf_length) == NO_ERROR, 
			"Unable to access network interfaces"))
		{
			std::free(p_adapter_info);
			return false;
		}

		// Create combined mac address hash
		PIP_ADAPTER_INFO adapter = p_adapter_info;
		for (adapter = p_adapter_info; adapter != nullptr; adapter = adapter->Next)
		{
			// Skip loopback device
			if(adapter->Type == MIB_IF_TYPE_LOOPBACK)
				continue;

			for (auto i = 0; i < adapter->AddressLength; i++)
				num_id += static_cast<uint64>(adapter->Address[i]) << (i * 8);
		}

		// Free adapter information
		std::free(p_adapter_info);

		//////////////////////////////////////////////////////////////////////////
		// UUID
		//////////////////////////////////////////////////////////////////////////

		static constexpr LPCTSTR key = "SOFTWARE\\Microsoft\\Cryptography";
		static constexpr LPCTSTR name = "MachineGuid";

		// simple RAII struct around registry key
		struct KeyHandle
		{
			KeyHandle(LPCTSTR key) : mKey(key)	{ }
			~KeyHandle()						{ if (mHandle != nullptr) { RegCloseKey(mHandle); } }
			bool open()							{ return RegOpenKeyEx(HKEY_LOCAL_MACHINE, mKey, 0, KEY_READ | KEY_WOW64_64KEY, &mHandle) == ERROR_SUCCESS; }

			HKEY mHandle = nullptr;
			LPCTSTR mKey = nullptr;
		};

		// Get key
		KeyHandle uuid_key(key);
		if (!error.check(uuid_key.open(), "Could not open registry key"))
			return false;

		// Extract value
		DWORD type; DWORD buf_size;
		if (!error.check(RegQueryValueEx(uuid_key.mHandle, name, NULL, &type, NULL, &buf_size) == ERROR_SUCCESS,
			"Could not read registry value"))
			return false;

		// Ensure type
		if (!error.check(type == REG_SZ, "Incorrect registry value type"))
			return false;

		// Get value
		std::string id_str(buf_size, '\0');
		assert(sizeof(BYTE) == sizeof(std::string::value_type));
		if (!error.check(RegQueryValueEx(uuid_key.mHandle, name, NULL, NULL, (PBYTE)(&id_str[0]), &buf_size) == ERROR_SUCCESS,
			"Could not read registry value"))
			return false;

        // hash and encode
        std::string hashed_id = utility::sha256(id_str);
		std::string id_encoded = utility::encode64(hashed_id);
        assert(id_encoded.size() >= 10);

        // convert to hex and truncate for readability
        std::stringstream ss;
        for(int i=0; i<10; ++i)
            ss << std::uppercase << std::hex << (int)id_encoded[i];
        outID = ss.str();

		return true;
	}
#else
	static bool generateMachineID(std::string& outID, nap::utility::ErrorState& error)
	{
		error.fail("Platform not supported");
		return false;
	}
#endif 


	//////////////////////////////////////////////////////////////////////////
	// LicenseService
	//////////////////////////////////////////////////////////////////////////

	LicenseService::LicenseService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	bool LicenseService::validateLicense(const nap::PublicKey& publicKey, LicenseInformation& outInformation, utility::ErrorState& error)
	{
		return validateLicense(publicKey.getKey(), nap::ESigningScheme::SHA256, outInformation, error);
	}


	bool LicenseService::validateLicense(const std::string& publicKey, LicenseInformation& outInformation, utility::ErrorState& error)
	{
		return validateLicense(publicKey, nap::ESigningScheme::SHA256, outInformation, error);
	}


	bool LicenseService::validateLicense(const nap::PublicKey& publicKey, nap::ESigningScheme signingScheme, LicenseInformation& outInformation, utility::ErrorState& error)
	{
		return validateLicense(publicKey.getKey(), signingScheme, outInformation, error);
	}


	bool LicenseService::validateLicense(const std::string& publicKey, nap::ESigningScheme signingScheme, LicenseInformation& outInformation, utility::ErrorState& error)
	{
		// Ensure the user provided a license
		if (!error.check(hasLicense(), "No .%s file found in: %s", licenseExtension, mDirectory.c_str()))
			return false;
		assert(utility::fileExists(mLicense));

		// Ensure the user provided a key
		if (!error.check(hasKey(), "No .%s file found in: %s", keyExtension, mDirectory.c_str()))
			return false;
		assert(utility::fileExists(mSignature));

        // Read license from disk
        std::ifstream license_stream(mLicense);
        std::stringstream license_content;
        license_content << license_stream.rdbuf();
        license_stream.close();

        // Read signature from disk
        std::ifstream signature_stream(mSignature);
        std::stringstream signature_content;
        signature_content << signature_stream.rdbuf();
        signature_stream.close();

		// Verify license using provided public application key
		if (!error.check(utility::verifyMessage(publicKey, license_content.str(), signingScheme, signature_content.str()), "Signature verification failed"))
			return false;

		// Remove first part
        std::string user_license = license_content.str();
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
		setArgument(arguments, "id", outInformation.mID);

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

		// If an id is provided, make sure it matches
		it = arguments.find("id");
		if (it != arguments.end())
		{
			std::string machine_id;
			if (!getMachineID(machine_id, error))
			{
				error.fail("Unable to generate machine identifier");
				return false;
			}

			if (!error.check(machine_id == it->second,"Identification failed: machine IDs don't match"))
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


    bool LicenseService::getMachineID(std::string& id, nap::utility::ErrorState& error)
    {
		return generateMachineID(id, error);
    }


    //////////////////////////////////////////////////////////////////////////
	// LicenseInformation
	//////////////////////////////////////////////////////////////////////////

	bool LicenseInformation::expired()
	{
		return this->canExpire() ? getCurrentTime() > mTime.getTimeStamp() : false;
	}
}
