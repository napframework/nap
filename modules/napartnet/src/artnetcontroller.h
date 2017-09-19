#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>

namespace nap
{
	using ArtNetNode = void*;

	class ArtNetService;
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

		// Initialization
		bool init(nap::utility::ErrorState& errorState);

		void send(const FloatChannelData& channelData, int channelOffset = 0);
		void send(const ByteChannelData& channelData, int channelOffset = 0);

		Address getAddress() const { return (mSubnet << 4) | mUniverse; }

	private:
		ArtNetNode getNode() const { return mNode; }

	public:
		uint8_t				mSubnet = 0;
		uint8_t				mUniverse = 0;

	private:
		friend class ArtNetService;

		ArtNetService*		mService = nullptr;
		ArtNetNode			mNode;
	};

	using ArtNetNodeCreator = rtti::ObjectCreator<ArtNetController, ArtNetService>;
}