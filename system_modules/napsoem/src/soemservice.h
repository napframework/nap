/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	/**
	 * Manages the soem (simple open ethercat master) library.
	 * IMPORTANT: winpcap is required on windows. Download it here: https://www.winpcap.org/
	 * The SOEM module builds against the winpcap library but does not ship with the dll or driver.
	 * Installing winpcap should be enough. Administator priviliges are not required.
	 */
	class NAPAPI SOEMService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 * Default constructor. This service has no settings associated with it.
		 */
		SOEMService(ServiceConfiguration* configuration);
		virtual ~SOEMService() override;

	protected:
		/**
		 * Prints all the available network adapters to console.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& error) override;
	};
}
