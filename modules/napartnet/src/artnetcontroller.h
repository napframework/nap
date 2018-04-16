#pragma once

// External Includes
#include <rtti/factory.h>
#include <nap/device.h>

namespace nap
{
	using ArtNetNode = void*;

	class ArtNetService;

	/**
	 * Creates an ArtNet address mapping so that data can be sent to it.
	 *
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

	private:

		/**
		 * @return libArtNet node, used for sending data
		 */
		ArtNetNode getNode() const { return mNode; }

	public:
		uint8_t				mSubnet = 0;									///< Subnet, in range from 0x0..0xF
		uint8_t				mUniverse = 0;									///< Universe, in range from 0x0..0xF
		int					mUpdateFrequency = getMaxUpdateFrequency();		///< Update artnet refresh rate, the default is the maximum refresh rate
		float				mWaitTime = 2.0f;								///< Number of seconds before the control data is send regardless of changes

	private:
		friend class ArtNetService;

		ArtNetService*		mService = nullptr;		// ArtNetService
		ArtNetNode			mNode = nullptr;		// libArtNet node for sending data
	};

	using ArtNetNodeCreator = rtti::ObjectCreator<ArtNetController, ArtNetService>;
}