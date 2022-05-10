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
	 * Base class of specific UDP client and server resources.  
	 * process() is automatically called by the thread this adapter links to.
	 * Both UDPClient & UDPServer extend UDPAdapter.
	 */
	class NAPAPI UDPAdapter : public Resource
	{
		friend class UDPThread;

		RTTI_ENABLE(Resource)
	public:
		ResourcePtr<UDPThread> mThread = nullptr; ///< Property: 'Thread' the udp thread the adapter registers itself to

		/**
		 * Initialization
		 * @param error contains error information
		 * @return true on success
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * called on destruction
		 */
		virtual void onDestroy() override;
    public:
        // Properties
        bool mAllowFailure 					= false;		///< Property: 'AllowFailure' if binding to socket is allowed to fail on initialization
	protected:
		/**
		 * called by a UDPThread
		 */
		virtual void process() = 0;

        bool handleAsioError(const asio::error_code& errorCode, utility::ErrorState& errorState, bool& success);
	};
}
