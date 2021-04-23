/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resource.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI UdpDevice : public Resource
	{
		friend class UdpThread;

		RTTI_ENABLE(Resource)
	protected:
		virtual void process() = 0;
	};
}
