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
		SHA1	= 0,		//< "SHA1"
		SHA224	= 1,		//< "SHA224"
		SHA256	= 2,		//< "SHA256"
		SHA384	= 3,		//< "SHA384"
		SHA512	= 4,		//< "SHA512"
	};
}
