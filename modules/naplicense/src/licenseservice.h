/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "publickey.h"
#include "signingscheme.h"
 
 // External Includes
#include <nap/service.h>
#include <nap/datetime.h>

namespace nap
{
	// Forward Declares
	class LicenseService;

	/**
	 * Tells the 'LicenseService' where to find the license and key (signature) files
	 * The license and key must be generated using the 'licensegenerator'
	 */
	class NAPAPI LicenseConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)
	public:
		std::string mDirectory = "{PROJECT_DIR}/license";		///< Property: 'LicenseDirectory' directory that contains the .license and .key files 
		virtual rtti::TypeInfo getServiceType() const override	{ return RTTI_OF(LicenseService); }
	};


	/**
	 * Validated user license information
	 */
	struct NAPAPI LicenseInformation
	{
		std::string			mName;					///< Extracted user name
		std::string			mMail;					///< Extracted user mail
		std::string			mApp;					///< Extracted application signature
		std::string			mTag;					///< Extracted additional license information
		std::string			mID;					///< Extracted machine id
		bool				mExpires = false;		///< If this license can expire
		DateTime			mTime;					///< License expiry date as system timestamp

		/**
		 * @return if the license can expire
		 */
		bool canExpire() const { return mExpires; }

		/**
		 * Returns if the license expired. Always false when the license has no expiration date
		 * Use it at runtime to check if the license is still valid.
		 * @return if this license expired, false when the license has no expiration date
		 */
		bool expired();
	};


	/**
	 * Validates a license using a public key.
	 *
	 * Call LicenseService::validateLicense() on initialization to check if the application has a valid license.
	 * You can generate a public / private RSA key pair using the 'keygen' tool, use
	 * the private key to generate a license using the 'licensegenerator' tool.
	 *
	 * On initialization the service looks for a '.key' and '.license' file in the directory 
	 * provided by the nap::LicenseConfiguration, defaults to: {PROJECT_DIR}/license.
	 *
	 * Note that on initialization no check is performed, you have to call 'validateLicense()',
	 * on app initialization, to verify the license. Based on the outcome it us up to you to
	 * implement the required security measures.
	 * 
	 * This is by no means a fail-safe licensing system, nothing is. It does however provide you 
	 * with a good layer of initial protection. If you want a more fail safe solution,
	 * consider using a license server instead. NAP however will not be able to provide that functionality,
	 * considering many NAP application must run stand-alone without an internet connection.
	 *
	 * It is recommended to compile the public key into your application as a string or into your
	 * application module as a 'nap::PublicKey'.
	 */
	class NAPAPI LicenseService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 *	Default constructor
		 */
		LicenseService(ServiceConfiguration* configuration);

        /**
         * Generates an ID for this machine, which can be used as 'id' input for the license generator.
         * The ID is a hash, created using the MAC addresses of the network interfaces + OS machine identifier.
         * Note that the returned ID is not required to be unique, it is a combination of various components but difficult to spoof.
		 * The ID is generated every time this function is called, cache it if read frequently. 
		 * 
         * @param id the generated machine ID
         * @param error contains the error if generation failed
         * @return if generation succeeded
         */
         bool getMachineID(uint64& id, nap::utility::ErrorState& error);

		/**
		 * Validates the user provided license using a public RSA key.
		 * Call this somewhere in your application, preferably on init(), to ensure the application has a valid license.
		 *
		 * The license is valid when:
		 * - a .license and .key file is found on service initialization (using the LicenseConfiguration)
		 * - is can be verified using the provided public key
		 * - it is not expired (if specified)
		 * - it has a matching device id (if specified)
		 *
		 * Note that a license, without an expiration date, is considered valid when it passes verification.
		 * It is up to the owner of the application to create and sign a license with an expiration date if required.
		 *
		 * Note that a license, without a machine identifier, is considered valid when it passes verification.
		 * It is up to the owner of the application to create and sign a license with a device id if required.
		 *
		 * @param publicKey public key, generated using the 'licensegenerator'
		 * @param outInformation validated user license information, empty if license is invalid
		 * @param error explains why the license is not valid
		 * @return if this app has a valid license
		 */
		bool validateLicense(const nap::PublicKey& publicKey, LicenseInformation& outInformation, utility::ErrorState& error);

		/**
		 * Validates the user provided license using a public RSA key.
		 * Call this somewhere in your application, preferably on init(), to ensure the application has a valid license.
		 *
		 * The license is valid when:
		 * - a .license and .key file is found on initialization (using the LicenseConfiguration)
		 * - is can be verified using the provided public key
		 * - it is not expired (if specified)
		 * - it has a matching device id (if specified)
		 *
		 * Note that a license, without an expiration date, is considered valid when it passes verification.
		 * It is up to the owner of the application to create and sign a license with an expiration date if required.
		 *
		 * Note that a license, without a machine identifier, is considered valid when it passes verification.
		 * It is up to the owner of the application to create and sign a license with a device id if required.
		 *
		 * @param publicKey public key, generated using the 'licensegenerator'
		 * @param outInformation validated user license information, empty if license is invalid
		 * @param error explains why the license is not valid
		 * @return if this app has a valid license
		 */
		bool validateLicense(const std::string& publicKey, LicenseInformation& outInformation, utility::ErrorState& error);

		/**
		 * Validates the user provided license using a public RSA key.
		 * Call this somewhere in your application, preferably on init(), to ensure the application has a valid license.
		 *
		 * The license is valid when:
		 * - a .license and .key file is found on service initialization (using the LicenseConfiguration)
		 * - is can be verified using the provided public key
		 * - it is not expired (if specified)
		 * - it has a matching device id (if specified)
		 *
		 * Note that a license, without an expiration date, is considered valid when it passes verification.
		 * It is up to the owner of the application to create and sign a license with an expiration date if required.
		 *
		 * Note that a license, without a machine identifier, is considered valid when it passes verification.
		 * It is up to the owner of the application to create and sign a license with a device id if required.
		 *
		 * @param publicKey public key, generated using the 'licensegenerator'
		 * @param signingScheme signing scheme used during license creation
		 * @param outInformation validated user license information, empty if license is invalid
		 * @param error explains why the license is not valid
		 * @return if this app has a valid license
		 */
		bool validateLicense(const nap::PublicKey& publicKey, nap::ESigningScheme signingScheme, LicenseInformation& outInformation, utility::ErrorState& error);

		/**
		 * Validates the user provided license using a public RSA key.
		 * Call this somewhere in your application, preferably on init(), to ensure the application has a valid license.
		 *
		 * The license is valid when:
		 * - a .license and .key file is found on initialization (using the LicenseConfiguration)
		 * - is can be verified using the provided public key
		 * - it is not expired (if specified)
		 * - it has a matching device id (if specified)
		 *
		 * Note that a license, without an expiration date, is considered valid when it passes verification.
		 * It is up to the owner of the application to create and sign a license with an expiration date if required.
		 *
		 * Note that a license, without a machine identifier, is considered valid when it passes verification.
		 * It is up to the owner of the application to create and sign a license with a device id if required.
		 *
		 * @param publicKey public key, generated using the 'licensegenerator'
		 * @param signingScheme signing scheme used during license creation
		 * @param outInformation validated user license information, empty if license is invalid
		 * @param error explains why the license is not valid
		 * @return if this app has a valid license
		 */
		bool validateLicense(const std::string& publicKey, nap::ESigningScheme signingScheme, LicenseInformation& outInformation, utility::ErrorState& error);

		/**
		 * Returns if the user provided a license. Does not mean it is valid.
		 * @return if a license is provided by the user.
		 */
		bool hasLicense() const						{ return !mLicense.empty(); }

		/**
		 * @return license file path
		 */
		const std::string& getLicense() const		{ return mLicense; }

		/**
		 * Returns if the user provided a license key (signature). Does not mean it is valid.
		 * @return if a key (signature) is provided by the user
		 */
		bool hasKey() const							{ return !mSignature.empty(); }

		/**
		 * @return key (signature) file path
		 */
		const std::string& getKey() const			{ return mSignature; }

	protected:
		/**
		 * Initializes the license service.
		 * Note that it does not check license validity at this point, you
		 * have to do that yourself by calling validateLicense(), together with a public RSA key.
		 * @param error contains the error if initialization fails
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& error) override;

	private:
		std::string mLicense;			///< Human readable license file path
		std::string mSignature;			///< License signature file path
		std::string mDirectory;			///< Directory that contains the licenses
	};
}
