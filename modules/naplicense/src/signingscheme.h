#pragma once

namespace nap
{
	/**
	 * Signing schemes. Used by the nap::LicenseService to validate a license.  
	 * Signing scheme used to generate a license with the 'licensegenerator' tool must
	 * match the signing scheme passed to the nap::LicenseService.
	 */
	enum class NAPAPI SigningScheme
	{
		RSASS_PKCS1v15_SHA1,
		RSASS_PKCS1v15_SHA224,
		RSASS_PKCS1v15_SHA256,
		RSASS_PKCS1v15_SHA384,
		RSASS_PKCS1v15_SHA512,
	};
}
