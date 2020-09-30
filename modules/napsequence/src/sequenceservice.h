/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sequenceplayer.h"

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>
#include <rtti/factory.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// forward declares
	class SequenceEventReceiver;

	/**
	 * Main interface for processing sequence outputs
	 */
	class NAPAPI SequenceService : public Service
	{
		friend class SequencePlayerOutput;

		RTTI_ENABLE(Service)
	public:
		/**
		 * Constructor
		 */
		SequenceService(ServiceConfiguration* configuration);

		/**
		 * Deconstructor
		 */
		virtual ~SequenceService();

		/**
		 * registers object creator method that can be passed on to the rtti factory
		 * @param objectCreator unique pointer to method
		 */
		static bool registerObjectCreator(std::unique_ptr<rtti::IObjectCreator>(*objectCreator)(SequenceService*));
	protected:
		/**
		 * registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * initializes service
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * updates any outputs
		 * @param deltaTime deltaTime
		 */
		virtual void update(double deltaTime) override;
	private:
		/**
		 * registers an output
		 * @param input reference to output
		 */
		void registerOutput(SequencePlayerOutput& output);

		/**
		 * removes an output
		 * @param input reference to input
		 */
		void removeOutput(SequencePlayerOutput& output);

		// vector holding raw pointers to outputs
		std::vector<SequencePlayerOutput*> mOutputs;
	};
}
