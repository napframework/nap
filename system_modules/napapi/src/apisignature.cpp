/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "apisignature.h"

// nap::apisignature run time class definition 
RTTI_BEGIN_CLASS(nap::APISignature, "Interface of an API method")
	RTTI_PROPERTY("Arguments",  &nap::APISignature::mArguments, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded, "Method input argument values")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	APISignature::~APISignature()			{ }
}
