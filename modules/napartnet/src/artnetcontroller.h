/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/factory.h>
#include <nap/device.h>
#include <future>
#include <nap/numeric.h>
#include <condition_variable>
#include <mutex>
#include <nap/timer.h>

namespace nap
{
	using ArtNetNode = void*;
	
	// Forward Declares
	class ArtNetService;

	namespace artnet
	{
		// This maximum is defined as the maximum refresh rate that can be achieved on the DMX512-A
		// physical layer with a full 512 channel(data slot) payload. The actual value is 44 packets per second.
		inline constexpr int refreshRate = 44;	///< The max supported artnet refresh rate
	}


	/**
	 * Mode used by an artnet controller to send data.
	 * When set to broadcast the message is broadcasted over the network.
	 * When set to unicast the message is only sent to compatible nodes.
	 * Compatible nodes match the subnet and universe of the controller.
	 */
	enum class EArtnetMode : int
	{
		Broadcast	= 0,			///< Artnet data is broadcasted over the network
		Unicast		= 1				///< Artnet data is sent only to compatible nodes that share the same universe and subnet
	};


	/**
	 * Creates an ArtNet controller node. 
	 * A controller node is used to convert dmx data into artnet data that is send over the network.
	 * Every controller node has a subnet and universe associated with it.
	 * See comments in ArtNetService on addressing on how data is eventually sent over the network.
	 */
	class NAPAPI ArtNetController : public Device
	{
		RTTI_ENABLE(Device)

	public:
		using ByteChannelData = std::vector<uint8_t>;
		using FloatChannelData = std::vector<float>;
		using Address = uint8_t;

		// Default constructor
		ArtNetController() = default;

		// Constructor used by factory
		ArtNetController(ArtNetService& service);

		/**
		 * Creates a mapping to the subnet and address.
		 * @param errorState Contains error information in case the function returns false.
		 * @return true on success, false otherwise. In case of an error, errorState contains error information.
		 */
		virtual bool start(nap::utility::ErrorState& errorState) override;

		/**
		 * Removes the controller from the service and destroys the managed artnet node
		 */
		virtual void stop() override;

		/**
		 * Sends normalized float channel data (ranging from 0.0 to 1.0) over the artnet network. Internally, the float data
		 * is converted to bytes. The actual sending is deferred until the update within the service, where data is sent when needed.
		 * @param channelData data to send in normalized floats (0.0 to 1.0)
		 * @param channelOffset defines where to insert the data in the array.
		 * If the channel offset plus the size of the channelData exceeds the maximum amount of channels per universe (512), the function will assert.
		 */
		void send(const FloatChannelData& channelData, int channelOffset = 0);

		/**
		 * Sends normalized float channel data (ranging from 0.0 to 1.0) over the artnet network. Internally, the float data
		 * is converted to bytes. The actual sending is deferred until the update, where data is sent when needed.
		 * @param channelData Channel data in normalized floats (0.0 to 1.0)
		 * @param channel The target channel where channelData should be applied to. Must be between 0 and 511.
		 */
		void send(float channelData, int channel);

		/**
		 * Sends byte channel data over the artnet network. The actual sending is deferred until the update of the service, where data is sent when needed.
		 * @param channelData data in unsigned bytes (0 - 255)
		 * @param channelOffset defines where to insert the data in the array.
		 * If the channel offset plus the size of the channelData exceeds the maximum amount of channels per universe (512), the function will assert.
		 */
		void send(const ByteChannelData& channelData, int channelOffset = 0);

		/**
		 * Sends byte channel data over the artnet network. The actual sending is deferred until the update, where data is sent when needed.
		 * @param channelData Channel data in unsigned bytes (0 - 255)
		 * @param channel The target channel where channelData should be applied to. Must be between 0 and 511.
		 */
		void send(uint8_t channelData, int channel);

		/**
		 *	Clears all the data associated with this controller, ie: sets their values to 0
		 */
		void clear();

		/**
		 * @return Address where this controllers maps to. Upper 4 bits contain subnet, lower 4 bits contain universe.
		 */
		Address getAddress() const											{ return createAddress(mSubnet, mUniverse); }
		
		/**
		 * Creates a unique artnet address based on a subnet and universe
		 * @param subnet the artnet subnet address
		 * @param universe the artnet universe address
		 */
		static Address createAddress(uint8_t subnet, uint8_t universe);

		/**
		 * Converts a nap artnet address in to a subnet and universe
		 * @param address the artnet address to convert
		 * @param subnet the subnet part of the address
		 * @param universe the universe part of the address;
		 */
		static void convertAddress(Address address, uint8_t& subnet, uint8_t& universe);

		uint8_t				mSubnet = 0;									///< Property: 'Subnet' range from 0 - 15
		uint8_t				mUniverse = 0;									///< Property: 'Universe' range from 0 - 15
		int					mUpdateFrequency = artnet::refreshRate;			///< Property: 'Frequency' artnet refresh rate, the default is the maximum refresh rate
		float				mWaitTime = 2.0f;								///< Property: 'WaitTime' number of seconds before the control data is send regardless of changes
		EArtnetMode			mMode = EArtnetMode::Broadcast;					///< Property: 'Mode' artnet message mode, Broadcast or Unicast
		int					mUnicastLimit = 10;								///< Property: 'UnicastLimit' allowed number of unicast nodes before switching to broadcast mode. Only has effect when mode = Unicast
		bool				mVerbose = false;								///< Property: 'Verbose' prints artnet network traffic information to the console
		float				mReadTimeout = 2.0f;							///< Property: 'Timeout' poll network node read timeout, only used when mode is set to Unicast
		std::string			mIpAddress = "";								///< Property: 'IPAddress' this controller's IP Address, when left empty the first available ethernet adapter is chosen.

	private:

		friend class ArtNetService;

		/**
		 * @return libArtNet node, used for sending data
		 */
		ArtNetNode getNode() const { return mNode; }

		/**
		 * Sends out a poll request and reads / interprets node information from the network
		 * This runs deferred as a task when mode = Unicast
		 * Ensures the list of available nodes to send data to remains up to date
		 */
		void exePollTask();

		/**
		 * Stops the background task from reading Artnet information of the network
		 * Called automatically when stopping this device.
		 */
		void stopPolling();

		/**
		 * Starts the background task of polling the network for available artnet node information
		 * This happens at a fixed interval based on the TimeOut variable
		 */
		void startPolling();

		/**
		 *	Actually performs the poll request
		 */
		void poll();

		/**
		 * Update called by service on main thread
		 */
		void update(double deltaTime);

		ArtNetService*				mService = nullptr;								///< ArtNetService
		ArtNetNode					mNode = nullptr;								///< libArtNet node for sending data
		int							mFoundNodes = 0;								///< Total number of artnet nodes found on the network
		int							mSocketDescriptor = -1;							///< Artnet node socket descriptor

		// Polling
		std::future<void>			mReadTask;										///< Task that updates available nodes on the network
		std::condition_variable		mConditionVar;									///< Used for telling the polling task to continue
		std::mutex					mPollMutex;										///< Used for locking critical resources
		nap::SystemTimer			mPollTimer;										///< Send out a poll request every 3 seconds (as dictated by the artnet standard)
		bool						mPoll = true;									///< Perform a poll operation on separate thread
		std::atomic<bool>			mExit = { false };								///< Cancel all running operations and exit task
	};

	using ArtNetNodeCreator = rtti::ObjectCreator<ArtNetController, ArtNetService>;
}
