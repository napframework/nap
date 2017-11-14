#include "meshtoartnetcomponent.h"

// External Includes
#include <nap/entity.h>
#include <meshutils.h>

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
		int tri_count = getTriangleCount(mMesh->getMeshInstance());

		TriangleDataPointer<glm::vec4> tri_color;
		TriangleDataPointer<int> tri_channel;
		TriangleDataPointer<int> tri_universe;
		TriangleDataPointer<int> tri_subnet;

		// Color attribute we use to sample
		nap::VertexAttribute<glm::vec4>& color_attr = mMesh->getColorAttribute();
		nap::VertexAttribute<int>& channel_attr = mMesh->getChannelAttribute();
		nap::VertexAttribute<int>& universe_attr = mMesh->getUniverseAttribute();
		nap::VertexAttribute<int>& subnet_attr = mMesh->getSubnetAttribute();

		for (int i = 0; i < tri_count; i++)
		{
			// Fetch attributes
			getTriangleValues(mMesh->getMeshInstance(), i, color_attr, tri_color);
			getTriangleValues(mMesh->getMeshInstance(), i, universe_attr, tri_universe);
			getTriangleValues(mMesh->getMeshInstance(), i, channel_attr, tri_channel);
			getTriangleValues(mMesh->getMeshInstance(), i, subnet_attr, tri_subnet);

			int universe = *(tri_universe[0]);
			int channel  = *(tri_channel[0]);
			int subnet   = *(tri_subnet[0]);
		
			// Find matching controller
			nap::ArtNetController*& controller = mControllers[ArtNetController::createAddress(subnet, universe)];

			// Send dmx values
			glm::vec4 color = *(tri_color[0]);
			std::vector<uint8> data
			{
				static_cast<uint8>(pow(color.r,1.0) * 255.0f),
				static_cast<uint8>(pow(color.g,1.0) * 255.0f),
				static_cast<uint8>(pow(color.b,1.0) * 255.0f),
				0
			};

			// Send to controller
			controller->send(data, channel);
		}
	}
}