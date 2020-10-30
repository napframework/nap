/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequenceplayeroutput.h"
#include "sequenceservice.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerOutput)
RTTI_END_CLASS

namespace  nap
{
	SequencePlayerOutput::SequencePlayerOutput(SequenceService& service)
		: mService(&service)
	{
	}


	bool SequencePlayerOutput::init(utility::ErrorState& errorState)
	{
		mService->registerOutput(*this);

		return true;
	}


	void SequencePlayerOutput::onDestroy()
	{
		mService->removeOutput(*this);
	}
}
