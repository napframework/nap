#pragma once

// External Includes
#include <rtti/factory.h>
#include <nap/device.h>
#include <future>
#include <nap/numeric.h>
#include <condition_variable>
#include <mutex>
#include <utility/datetimeutils.h>

namespace nap
{
	using ArtNetNode = void*;
	
	// Forward Declares
	class ArtNetService;

	/**
	 * Mode used by an artnet controller to send data.
	 * When set to broadcast the message is broadcasted over the network.
	 * When set to unicast the message is only send to compatible nodes.
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

		virtual ~ArtNetController() override;

		/**
		 * Creates a mapping to the subnet and address.
		 * @param errorState Contains error information in case the function returns false.
		 * @return true on success, false otherwise. In case of an error, @errorState contains error information.
		 */
		virtual bool start(nap::utility::ErrorState& errorState) override;

		/**
		 * Removes the controller from the service and destroys the managed artnet node
		 */
		virtual void stop() override;

		/**
		 * Sends normalized float channel data (ranging from 0.0 to 1.0) over the artnet network. Internally, the float data
		 * is converted to bytes. The actual sending is deferred until the update within the service, where data is sent when needed.
		 * @param channelData Channel data in normalized floats (0.0 to 1.0)
		 * @param channelOffset Channel offset, the target start channel where @channelData should be applied to. 
		 * If the channel offset plus the size of the @channelData exceeds the maximum amount of channels per universe (512), the function will assert.
		 */
		void send(const FloatChannelData& channelData, int channelOffset = 0);

		/**
		 * Sends normalized float channel data (ranging from 0.0 to 1.0) over the artnet network. Internally, the float data
		 * is converted to bytes. The actual sending is deferred until the update, where data is sent when needed.
		 * @param channelData Channel data in normalized floats (0.0 to 1.0)
		 * @param channel The target channel where @channelData should be applied to. Must be between 0 and 512.
		 */
		void send(float channelData, int channel);

		/**
		 * Sends byte channel data over the artnet network. The actual sending is deferred until the update of the service, where data is sent when needed.
		 * @param channelData Channel data in unsigned bytes (0 - 255)
		 * @param channelOffset Channel offset, the target start channel where @channelData should be applied to. 
		 * If the channel offset plus the size of the @channelData exceeds the maximum amount of channels per universe (512), the function will assert.
		 */
		void send(const ByteChannelData& channelData, int channelOffset = 0);

		/**
		 * Sends byte channel data over the artnet network. The actual sending is deferred until the update, where data is sent when needed.
		 * @param channelData Channel data in unsigned bytes (0 - 255)
		 * @param channel The target channel where @channelData should be applied to. Must be between 0 and 512.
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

		/**
		 * @return the max update frequency
		 * 44hz, see http ://art-net.org.uk/wordpress/?page_id=456 / Refresh Rate)
		 */
		static const int getMaxUpdateFrequency();

		uint8_t				mSubnet = 0;									///< Subnet, in range from 0 - 15
		uint8_t				mUniverse = 0;									///< Universe, in range from 0 - 15
		int					mUpdateFrequency = getMaxUpdateFrequency();		///< Update artnet refresh rate, the default is the maximum refresh rate
		float				mWaitTime = 2.0f;								///< Number of seconds before the control data is send regardless of changes
		EArtnetMode			mMode = EArtnetMode::Broadcast;					///< Artnet message mode
		int					mUnicastLimit = 10;								///< Allowed number of unicast nodes before switching to broadcast mode. Only has effect when mode = Unicast
		float				mTimeOut = 1.0f;								///< Timeout in seconds when polling network for node activity
		bool				mVerbose;										///< Prints artnet network traffic information to the consolve

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
		void pollAndRead();

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
		 * Update called by service on main thread
		 */
		void update(double deltaTime);

		ArtNetService*				mService = nullptr;								///< ArtNetService
		ArtNetNode					mNode = nullptr;								///< libArtNet node for sending data
		int							mFoundNodes = 0;								///< Total number of artnet nodes found on the network
		int							mSocketDescriptor = -1;							///< Artnet node socket descriptor

		// Polling
		std::future<void>			mReadTask;										///< Task that updates available nodes on the network
		std::condition_variable		mConditionVar;
		std::mutex					mPollMutex;
		nap::utility::SystemTimer	mPollTimer;
		std::atomic<bool>			mRead = false;									///< Perform a read operation on the main thread
		bool						mPoll = true;									///< Perform a poll operation on separate thread
		bool						mExit = false;									///< Cancel all running operations and exit task
	};

	using ArtNetNodeCreator = rtti::ObjectCreator<ArtNetController, ArtNetService>;
}