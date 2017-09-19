#pragma once

// External Includes
#include <nap/service.h>
#include <nap/entity.h>

namespace nap
{
	class ArtNetController;

	/**
	 * Main interface for rendering to various Etherdream Dacs
	 * The service is responsible for opening / closing the general Etherdream library
	 * and allows for rendering data to the available dacs
	 */
	class NAPAPI ArtNetService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		using ByteChannelData = std::vector<uint8_t>;
		using FloatChannelData = std::vector<float>;

		// Default Constructor
		ArtNetService();

		// Default Destructor
		virtual ~ArtNetService();

		ArtNetService(const ArtNetService& rhs) = delete;
		ArtNetService& operator=(const ArtNetService& rhs) = delete;

		bool addController(ArtNetController& node, utility::ErrorState& errorState);
		void removeController(ArtNetController& node);

		void send(ArtNetController& inNode, const FloatChannelData& channelData, int channelOffset = 0);
		void send(ArtNetController& inNode, const ByteChannelData& channelData, int channelOffset = 0);

		void update();

		/**
		 *	Register specific object creators
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

	private:
		struct ControllerData
		{
			ArtNetController*			mController;
			ByteChannelData				mData;
			double						mLastUpdateTime;
			bool						mIsDirty;
		};
		using NodeKey = uint8_t;

		using NodeMap = std::unordered_map<NodeKey, std::unique_ptr<ControllerData>>;
		using DirtyNodeList = std::unordered_set<NodeKey>;

		NodeMap			mControllers;
		double			mUpdateFrequency = 1.0 / 44.0; // Update artnet channel data at 44 Hz (see http://art-net.org.uk/wordpress/?page_id=456 / Refresh Rate)
	};
}