#include "meshtoartnetcomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <color.h>
#include <mathutils.h>
#include <TriangleIterator.h>

// nap::meshtoartnetcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::MeshToArtnetComponent)
	RTTI_PROPERTY("Mesh",			&nap::MeshToArtnetComponent::mMesh,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Controllers",	&nap::MeshToArtnetComponent::mArtnetControllers,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::meshtoartnetcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MeshToArtnetComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void MeshToArtnetComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}

	bool MeshToArtnetComponentInstance::init(utility::ErrorState& errorState)
	{
		nap::MeshToArtnetComponent* resource = getComponent<MeshToArtnetComponent>();
		assert(resource != nullptr);

		// Register artnet controller, artnet controllers with duplicate addressess are not allowed
		for (auto& res_controller : resource->mArtnetControllers)
		{
			if (!errorState.check(mControllers.find(res_controller->getAddress())== mControllers.end(), "artnet controller: %s, already registered: %s", res_controller->mID.c_str(), this->mID.c_str()))
				return false;
			mControllers.emplace(std::make_pair(res_controller->getAddress(), res_controller.get()));
		}

		// Get the mesh
		mMesh = resource->mMesh.get();

		// Make sure all the artnet addresses on the mesh are associated with one of the controllers
		for (const auto& address : mMesh->getAddresses())
		{
			if (!errorState.check(mControllers.find(address) != mControllers.end(), "mesh: %s has an artnet address that's not tied to a controller: %s", mMesh->mID.c_str(), this->mID.c_str()))
				return false;
		}
		return true;
	}


	void MeshToArtnetComponentInstance::update(double deltaTime)
	{
		std::vector<glm::vec4>& artnet_data = mMesh->getArtnetColorAttribute().getData();
		std::vector<int>& channel_data		= mMesh->getChannelAttribute().getData();
		std::vector<int>& universe_data		= mMesh->getUniverseAttribute().getData();
		std::vector<int>& subnet_data		= mMesh->getSubnetAttribute().getData();

		TriangleShapeIterator shape_iterator(mMesh->getMeshInstance());
		while (!shape_iterator.isDone())
		{
			glm::ivec3 indices = shape_iterator.next();

			int universe = universe_data[indices[0]];
			int channel = channel_data[indices[0]];
			int subnet = subnet_data[indices[0]];
			glm::vec4 artnet_color = artnet_data[indices[0]];

			// Find matching controller
			nap::ArtNetController*& controller = mControllers[ArtNetController::createAddress(subnet, universe)];

			// Set data
			mArtnetData[0] = static_cast<uint8>(artnet_color.r * static_cast<float>(math::max<uint8>()));
			mArtnetData[1] = static_cast<uint8>(artnet_color.g * static_cast<float>(math::max<uint8>()));
			mArtnetData[2] = static_cast<uint8>(artnet_color.b * static_cast<float>(math::max<uint8>()));
			mArtnetData[3] = static_cast<uint8>(artnet_color.a * static_cast<float>(math::max<uint8>()));

			// Send to controller
			controller->send(mArtnetData, channel);
		}
	}
}