#include "meshtoartnetcomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>
#include <color.h>
#include <mathutils.h>
#include <triangleiterator.h>

// nap::meshtoartnetcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::MeshToArtnetComponent)
	RTTI_PROPERTY("Mesh",			&nap::MeshToArtnetComponent::mMesh,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Controllers",	&nap::MeshToArtnetComponent::mArtnetControllers,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MinArtnetValue",	&nap::MeshToArtnetComponent::mMinValue,				nap::rtti::EPropertyMetaData::Default)
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

		// Copy over min value, ensure value between 0 and 255
		if (!errorState.check(resource->mMinValue <= math::max<uint8>() && resource->mMinValue >= 0, "Artnet value needs to be between 0 and 255"))
			return false;
		mMinValue = static_cast<uint8>(resource->mMinValue);

		return true;
	}


	void MeshToArtnetComponentInstance::update(double deltaTime)
	{
		std::vector<glm::vec4>& color_data  = mMesh->getColorAttribute().getData();
		std::vector<int>& channel_data		= mMesh->getChannelAttribute().getData();
		std::vector<int>& universe_data		= mMesh->getUniverseAttribute().getData();
		std::vector<int>& subnet_data		= mMesh->getSubnetAttribute().getData();

		TriangleIterator triangle_iterator(mMesh->getMeshInstance());
		while (!triangle_iterator.isDone())
		{
			Triangle triangle = triangle_iterator.next();

			int universe = universe_data[triangle.firstIndex()];
			int channel = channel_data[triangle.firstIndex()];
			int subnet = subnet_data[triangle.firstIndex()];
			glm::vec4 artnet_color = color_data[triangle.firstIndex()];

			// Find matching controller
			nap::ArtNetController*& controller = mControllers[ArtNetController::createAddress(subnet, universe)];

			// Set data
			float max_artnet_value = static_cast<float>(math::max<uint8>());
			mArtnetData[0] = artnet_color.r <= nap::math::epsilon<float>() ? 0 :
				static_cast<uint8>(math::fit<float>(artnet_color.r, 0.0f, 1.0f, (float)mMinValue, max_artnet_value));

			mArtnetData[1] = artnet_color.g <= nap::math::epsilon<float>() ? 0 :
				static_cast<uint8>(math::fit<float>(artnet_color.g, 0.0f, 1.0f, (float)mMinValue, max_artnet_value));

			// Send to controller
			controller->send(mArtnetData, channel);
		}
	}
}