#pragma once

// External Includes
#include <nap/device.h>
#include <nap/numeric.h>
#include <nap/signalslot.h>
#include <future>
#include <atomic>

namespace nap
{
	/**
	 * ethercatmaster
	 */
	class NAPAPI EtherCATMaster : public Device
	{
		RTTI_ENABLE(Device)
	public:

		// Stops the device
		virtual ~EtherCATMaster() override;

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts the device
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the device
		 */
		virtual void stop() override;

		/**
		 * @return number of slaves on the network
		 */
		int getSlaveCount() const;

		std::string mAdapter;	///< Property: 'Adapter' the name of the ethernet adapter to use. A list of available adapters is printed by the SOEM service on startup.

	private:
		char mIOmap[4096];
		int  mExpectedWKC = 0;
		int  mActualWCK = 0;
		std::future<void> mTask;											///< The background server thread
		std::atomic<bool> mStopRunning = { false };							///< If the task should be stopped

		void run();

		void requestInitState();
	};
}
