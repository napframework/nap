#pragma once

#include <nap/service.h>
#include <nap/coreattributes.h>
#include <nap/configure.h>
#include "ofnledclient.h"

namespace nap
{
	/**
	 * @brief client interface to nled server
	 */
	class NLedService : public Service
	{
		RTTI_ENABLE_DERIVED_FROM(Service)
	public:
		/**
		 * Constructor
		 */
		NLedService() = default;

		/**
		 * Destructor
		 */
		virtual ~NLedService();

		/**
		 * Start running service
		 * This will force the service to connect
		 */
		void start();

		/**
		 * Stop running service
		 * This will force the service to disconnect
		 */
		void stop();

		/**
		 * Name of the nled server
		 */
		Attribute<std::string> serverName	{ this, "serverName", "raspberrypi" };
		
		/**
		 * Port number of the nled server app
		 */
		Attribute<int> portNumber	{ this, "portNumber",  7845 };

	protected:
		virtual void registerTypes(nap::Core& core);

	private:
		//@name Panels
		nofNLedClient mClient;
	};
}

RTTI_DECLARE(nap::NLedService)