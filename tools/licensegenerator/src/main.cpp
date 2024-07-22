#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <commandline.h>
#include <chrono>
#include <fstream>

#include "opensslapi.h"

using namespace std;
using SystemClock = std::chrono::system_clock;
using SystemTimeStamp = std::chrono::time_point<SystemClock>;

constexpr const char* licenceToken = "LICENSE@";
constexpr const char* licenseExtension = "license";
constexpr const char* keyExtension = "key";
constexpr const char* defaultSigningSceme = "SHA256";
constexpr const char* supportedSigningSchemes[] =
{
    "SHA1", "SHA224", defaultSigningSceme, "SHA384", "SHA512", ""
};

#define BUFFER_SIZE 2048
static unsigned char buffer[BUFFER_SIZE];

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
 * licensegenerator -k c:/keys/key.private -s SHA256 -f ben -l davis -a myapp -m ben@davis.com -d 30/12/2025 -t educational -o c:/license
 * ~~~~~
 */
int main(int argc, char *argv[])
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

    // Read privkey from disk
    std::ifstream priv_key_stream(commandLine.mKey);
    std::stringstream priv_key;
    priv_key << priv_key_stream.rdbuf();
    priv_key_stream.close();

    // Create signature
    std::string signature;
    if (!nap::openssl::createSignature(priv_key.str(), lic_content.str(), signingScheme, signature))
    {
        std::cout << "Failed to create signature" << std::endl;
        return -1;
    }

    // Save license
    std::ofstream lic_file(lic_loc.str());
    lic_file << lic_content.str();
    lic_file.close();

    // Save signature
    std::ofstream key_file(key_loc.str());
    key_file << signature;
    key_file.close();

    std::cout << "Successfully created and signed license" << std::endl;
    std::cout << "Key location:         " << key_loc.str() << std::endl;
    std::cout << "License location:     " << lic_loc.str() << std::endl;

    // All good
    return 0;
}
