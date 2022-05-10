/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace nap
{
	/**
	 * Signing schemes. Used by the nap::LicenseService to validate a license.  
	 * Signing scheme used to generate a license with the 'licensegenerator' tool must
	 * match the signing scheme passed to the nap::LicenseService.
	 */
	enum class ESigningScheme : int
	{
		RSASS_PKCS1v15_SHA1 = 0,
		RSASS_PKCS1v15_SHA224 = 1,
		RSASS_PKCS1v15_SHA256 = 2,
		RSASS_PKCS1v15_SHA384 = 3,
		RSASS_PKCS1v15_SHA512 = 4,
	};
}
