/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resourceptr.h>
#include <nap/resource.h>
#include <nap/device.h>
#include <thread>

// NAP includes
#include <nap/numeric.h>
#include <concurrentqueue.h>
#include <rtti/factory.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Different update methods of an UDPThread.
	 */
	enum EUDPThreadUpdateMethod : int
	{
		MAIN_THREAD			= 0,			///< process UDPAdapters on main thread
		SPAWN_OWN_THREAD	= 1,			///< process UDPAdapters in newly spawned thread
		MANUAL				= 2				///< only process UDPAdapters when the user explicitly calls manualProcess on the UDPThread
	};

	// forward declares
	class UDPAdapter;
	class UDPService;

	/**
	 * UDPThread is a device that calls the process() function on UDPAdapters.
	 * UDPAdapters are typically UDPServers or UDPClients.
	 * The UDPThread can can run on the main thread, in which case it registers itself to the UDPService,
	 * in that case the UDPThread will call process() of the registered adapters inside the update() call of the UDPService.
	 * The UDPThread can spawn its own thread, in which case process() will be called within the while loop of a newly spawned thread.
	 * When the user chooses to use MANUAL, process() will be called only when the user explicitly calls the manualProcess()
	 * function of the UDPThread
	 */
	class NAPAPI UDPThread : public Device
	{
		friend class UDPService;
		friend class UDPAdapter;

		RTTI_ENABLE(Device)
	public:
		/**
		 * Constructor
		 * @param service reference to UDP service
		 */
		UDPThread(UDPService& service);

		/**
		 * Starts the UDPThread, spawns new thread if necessary or registers to UDPService
		 * @param errorState contains any errors
		 * @return true on succes
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the UDPThread, stops own thread or removes itself from service
		 */
		virtual void stop() override;
	public:
		// properties
		EUDPThreadUpdateMethod mUpdateMethod = EUDPThreadUpdateMethod::MAIN_THREAD; ///< Property: 'Update Method' the way the UDPThread should process adapters

		/**
		 * Call this when update method is set to manual.
		 If the update method is MAIN_THREAD or SPAWN_OWN_THREAD, this function will not do anything.
		 */
		void manualProcess();
	private:
		/**
		 * the threaded function
		 */
		void thread();

		/**
		 * the process method, will call process on any registered adapter
		 */
		void process();

		/**
		 * registers an adapter. Thread-safe
		 * @param adapter the UDPAdapter to process
		 */
		void registerAdapter(UDPAdapter* adapter);

		/**
		 * removes an adapter. Thread-safe
		 * @param adapter the UDPAdapter to remove
		 */
		void removeAdapter(UDPAdapter* adapter);

		// threading
		std::thread 										mThread;
		std::mutex											mMutex;
		std::atomic_bool 									mRun = { false };
		std::function<void()> 								mManualProcessFunc;

		// service
		UDPService& 				mService;

		// adapters
		std::vector<UDPAdapter*> 	mAdapters;
	};

	// Object creator used for constructing the UDP thread
	using UDPThreadObjectCreator = rtti::ObjectCreator<UDPThread, UDPService>;
}
