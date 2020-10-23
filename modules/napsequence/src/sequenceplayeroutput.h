/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// nap includes
#include <nap/resource.h>
#include <rtti/factory.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// forward declars
	class SequenceService;

	/**
	 * SequencePlayerOutput is the base class for outputs for the sequenceplayer
	 * Outputs can be used by SequencePlayerAdapters to link tracks to objects.
	 * F.E SequencePlayerCurveOutput can link a parameter to a curve track
	 */
	class NAPAPI SequencePlayerOutput : public Resource
	{
		friend class SequenceService;

		RTTI_ENABLE(Resource)
	public:
		/**
		 * Constructor
		 * @param service reference to SequenceService
		 */
		SequencePlayerOutput(SequenceService& service);
        
        virtual ~SequencePlayerOutput(){};

		/**
		 * upon initialisation input registers itself to the service
		 * @param errorState contains any errors
		 * @return true if succeed
		 */
		virtual bool init(utility::ErrorState& errorState) override ;

		/**
		 * upon destruction, removes itself from service
		 */
		virtual void onDestroy() override ;
	protected:
		/**
		 * called from sequenceservice update loop main thread
		 * @param deltaTime time since last update call
		 */
		virtual void update(double deltaTime){};

		// ptr to service
		SequenceService* mService = nullptr;
	};
}
