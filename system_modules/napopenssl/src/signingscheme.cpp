/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "signingscheme.h"

// External Includes
#include <rtti/typeinfo.h>

// Orientation enum
RTTI_BEGIN_ENUM(nap::ESigningScheme)
	RTTI_ENUM_VALUE(nap::ESigningScheme::SHA1,    "SHA1"),
	RTTI_ENUM_VALUE(nap::ESigningScheme::SHA224,  "SHA224"),
	RTTI_ENUM_VALUE(nap::ESigningScheme::SHA256,  "SHA256"),
	RTTI_ENUM_VALUE(nap::ESigningScheme::SHA384,  "SHA384"),
	RTTI_ENUM_VALUE(nap::ESigningScheme::SHA512,  "SHA512")
RTTI_END_ENUM

namespace nap
{ }
