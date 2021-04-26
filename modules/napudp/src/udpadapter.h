/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <nap/resourceptr.h>
#include <udpthread.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * UDPAdapters process() function will be called from the UDPThread the UDPAdapter registers to.
	 * Both UDPClient & UDPServer extend on UDPAdapter
	 */
	class NAPAPI UDPAdapter : public Resource
	{
		friend class UDPThread;

		RTTI_ENABLE(Resource)
	public:
		ResourcePtr<UDPThread> mThread = nullptr; ///< Property: 'Thread' the udp thread the adapter registers itself to

		/**
		 * initialization
		 * @param error contains error information
		 * @return true on succes
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * called on destruction
		 */
		virtual void onDestroy() override;
	protected:
		/**
		 * called by a UDPThread
		 */
		virtual void process() = 0;
	};
}
