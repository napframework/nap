/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <entity.h>

namespace nap
{
	class ArtNetController;
	class ArtNetReceiver;
	class ArtNetInputComponentInstance;

	/**
	 * Service for sending and receiving data over Artnet. Data is natively sent using bytes values, but the service provides
	 * a convenience function to send data using normalized float values. When calling one of the send functions, the 
	 * data is not sent directly, this is deferred to the update() call, where the service can control the frequence of 
	 * sending data and where it can make sure that data is resent when needed. Because of this, updating universes 
	 * partially and incrementally using multiple send calls does not impact functionality or performance of the system.
	 *
	 * To send data, create an ArtNetController and specify the subnet and universe for the controller. Then call send on it,
	 * this will redirect the send call to this service.
	 *
	 * To receive data, first create an ArtNetReceiver and specify the port to listen on for receiving ArtDmx packets.
	 * Then add an ArtNetInputComponent and specify whether to filter on ArtDmx packets by Net, SubNet and Universe or
	 * to receive all packets. Connect to the input component's packetReceived slot for handling the incoming events.
	 *
	 * Note: in the current sending implementation, only subnet and universe can be addressed. A correct implementation should include
	 * the full 16 bits and be able to address net(7 bits) - subnet(4 bits) - universe (4 bits). As we are using a library
	 * that doesn't support this correctly, we cannot address multiple nets. Now we are using 8 bits for the address, this should
	 * change as soon as we have more control over the implementation.
	 */
	class NAPAPI ArtNetService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		using ByteChannelData = std::vector<uint8_t>;
		using FloatChannelData = std::vector<float>;

		// Default Constructor
		ArtNetService(ServiceConfiguration* configuration);

		// Default Destructor
		virtual ~ArtNetService();

		ArtNetService(const ArtNetService& rhs) = delete;
		ArtNetService& operator=(const ArtNetService& rhs) = delete;

	protected:
		/**
		 *	Register specific object creators
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		* Makes sure that data that is sent using the various send functions is transmitted over the network
		* and consume incoming ArtDmx packet events from Art-Net Receivers.
		*/
		virtual void update(double deltaTime) override;

	private:
		/**
		 * Adds a controller to the service. This should be called from start() and the return value should be tested to validate
		 * that adding of the controller was successful.
		 * @param controller Controller to add.
		 * @param errorState Out parameter that describes the error if the function returns false.
		 * @return Whether adding was successful. If not successful, @errorState contains error information.
		 */
		bool addController(ArtNetController& controller, utility::ErrorState& errorState);

		/**
		 * Removes a controller that was previously successfully added using @addController. This should be called from stop().
		 * @param controller Controller to remove.
		 */
		void removeController(ArtNetController& controller);

		/**
		 * Adds a receiver to the service. This should be called from start() and the return value should be tested to validate
		 * that adding of the receiver was successful.
		 * @param receiver ArtNetReceiver to add.
		 * @param errorState Out parameter that describes the error if the function returns false.
		 * @return Whether adding was successful. If not successful, @errorState contains error information.
		 */
		bool addReceiver(ArtNetReceiver& receiver, utility::ErrorState& errorState);

		/**
		 * Removes a receiver that was previously successfully added using @addReceiver. This should be called from stop().
		 * @param receiver ArtNetReceiver to remove.
		 */
		void removeReceiver(ArtNetReceiver& receiver);

		/**
		 * Add an Art-Net input component to the service, which will receive consumed ArtDmx packet events when its filters
		 * (Net, SubNet and Universe, or Receive All) match the properties of the incoming event.
		 * @param input The ArtNetInputComponentInstance to add.
		 */
		void addInputComponent(ArtNetInputComponentInstance& input);

		/**
		 * Remove an Art-Net input component from the service.
		 * @param input The ArtNetInputComponentInstance to remove.
		 */
		void removeInputComponent(ArtNetInputComponentInstance& input);

		/**
		 * Sends normalized float channel data (ranging from 0.0 to 1.0) over the artnet network. Internally, the float data
		 * is converted to bytes. The actual sending is deferred until the update, where data is sent when needed.
		 * @param controller Controller to send from, which specifies subnet and universe.
		 * @param channelData Channel data in normalized floats (0.0 to 1.0)
		 * @param channelOffset Channel offset, the target start channel where @channelData should be applied to. If the 
		 *                      channel offset plus the size of the @channelData exceeds the maximum amount of channels per 
		 *                      universe (512), the function will assert.
		 */
		void send(ArtNetController& controller, const FloatChannelData& channelData, int channelOffset = 0);

		/**
		 * Sends normalized float channel data (ranging from 0.0 to 1.0) over the artnet network. Internally, the float data
		 * is converted to bytes. The actual sending is deferred until the update, where data is sent when needed.
		 * @param controller Controller to send from, which specifies subnet and universe.
		 * @param channelData Channel data in normalized floats (0.0 to 1.0)
		 * @param channel The target channel where @channelData should be applied to. Must be between 0 and 512.
		 */
		void send(ArtNetController& controller, float channelData, int channel);

		/**
		 * Sends byte channel data over the artnet network. The actual sending is deferred until the update, where data is sent when needed.
		 * @param controller Controller to send from, which specifies subnet and universe.
		 * @param channelData Channel data in unsigned bytes (0 - 255)
		 * @param channelOffset Channel offset, the target start channel where @channelData should be applied to. If the
		 *                      channel offset plus the size of the @channelData exceeds the maximum amount of channels per
		 *                      universe (512), the function will assert.
		 */
		void send(ArtNetController& controller, const ByteChannelData& channelData, int channelOffset = 0);

		/**
		 * Sends byte channel data over the artnet network. The actual sending is deferred until the update, where data is sent when needed.
		 * @param controller Controller to send from, which specifies subnet and universe.
		 * @param channelData Channel data in unsigned bytes (0 - 255)
		 * @param channel The target channel where @channelData should be applied to. Must be between 0 and 512.
		 */
		void send(ArtNetController& controller, uint8_t channelData, int channel);

		/**
		 * Clears all data associated with a specific controller, ie: sets the controller buffer to 0
		 * @param controller the dmx controller to clear the values for
		 */
		void clear(ArtNetController& controller);

	private:
		friend class ArtNetController;
		friend class ArtNetReceiver;
		friend class ArtNetInputComponentInstance;

		struct ControllerData
		{
			ArtNetController*			mController;			// ArtNet controller specifying target subnet and universe
			ByteChannelData				mData;					// Byte data for all channels for a single controller
			double						mLastUpdateTime;		// Last time this controller was transmitted over the network
			bool						mIsDirty;				// Identifies whether this controller contains new data
		};

		using ControllerKey = uint8_t;							// The key is the target address (subnet-universe), which is currently limited to 8 bits as it does not include net (7 bits)
		using ReceiverKey = uint16_t;							// The key is the port which the receiver will listen to

		using ControllerMap = std::unordered_map<ControllerKey, std::unique_ptr<ControllerData>>;
		using ReceiverMap = std::unordered_map<ReceiverKey, ArtNetReceiver*>;
		using InputComponents = std::vector<ArtNetInputComponentInstance*>;
		using DirtyNodeList = std::unordered_set<ControllerKey>;

		ControllerMap					mControllers;			// Controller map that maps an absolute controller address to a controller
		ReceiverMap						mReceivers;				// Receiver map that maps a receiver port to a receiver
		InputComponents					mInputs;				// All the Art-Net input components registered to the service
	};
}
