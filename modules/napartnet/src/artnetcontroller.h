#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>

namespace nap
{
	using ArtNetNode = void*;

	class ArtNetService;

	/**
	 * Creates an ArtNet address mapping so that data can be sent to it.
	 *
	 * See comments in ArtNetService on addressing on how data is eventually sent over the network.
	 */
	class NAPAPI ArtNetController : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		using ByteChannelData = std::vector<uint8_t>;
		using FloatChannelData = std::vector<float>;
		using Address = uint8_t;

		// Default constructor
		ArtNetController() = default;

		// Constructor used by factory
		ArtNetController(ArtNetService& service);

		~ArtNetController();

		/**
		 * Creates a mapping to the subnet and address. 
		 * @param errorState Contains error information in case the function returns false.
		 * @return true on success, false otherwise. In case of an error, @errorState contains error information.
		 */
		bool init(nap::utility::ErrorState& errorState);

		/**
		 * Sends normalized float channel data (ranging from 0.0 to 1.0) over the artnet network. Internally, the float data
		 * is converted to bytes. The actual sending is deferred until the update within the service, where data is sent when needed.
		 * @param channelData Channel data in normalized floats (0.0 to 1.0)
		 * @param channelOffset Channel offset, the target start channel where @channelData should be applied to. If the
		 *                      channel offset plus the size of the @channelData exceeds the maximum amount of channels per
		 *                      universe (512), the function will assert.
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
		 * @param channelOffset Channel offset, the target start channel where @channelData should be applied to. If the
		 *                      channel offset plus the size of the @channelData exceeds the maximum amount of channels per
		 *                      universe (512), the function will assert.
		 */
		void send(const ByteChannelData& channelData, int channelOffset = 0);

		/**
		 * Sends byte channel data over the artnet network. The actual sending is deferred until the update, where data is sent when needed.
		 * @param channelData Channel data in unsigned bytes (0 - 255)
		 * @param channel The target channel where @channelData should be applied to. Must be between 0 and 512.
		 */
		void send(uint8_t channelData, int channel);

		/**
		 * @return Address where this controllers maps to. Upper 4 bits contain subnet, lower 4 bits contain universe.
		 */
		Address getAddress() const { return (mSubnet << 4) | mUniverse; }

	private:

		/**
		 * @return libArtNet node, used for sending data
		 */
		ArtNetNode getNode() const { return mNode; }

	public:
		uint8_t				mSubnet = 0;			///< Subnet, in range from 0x0..0xF
		uint8_t				mUniverse = 0;			///< Universe, in range from 0x0..0xF

	private:
		friend class ArtNetService;

		ArtNetService*		mService = nullptr;		// ArtNetService
		ArtNetNode			mNode;					// libArtNet node for sending data
	};

	using ArtNetNodeCreator = rtti::ObjectCreator<ArtNetController, ArtNetService>;
}