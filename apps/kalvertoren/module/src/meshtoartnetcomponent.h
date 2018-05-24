#pragma once

// Local includes
#include "artnetmeshfromfile.h"

// External includes
#include <component.h>
#include <artnetcontroller.h>
#include <nap/resourceptr.h>

namespace nap
{
	class MeshToArtnetComponentInstance;

	/**
	 *	meshtoartnetcomponent
	 */
	class NAPAPI MeshToArtnetComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(MeshToArtnetComponent, MeshToArtnetComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		//property: Object pointer to the mesh that holds the values we want to convert and send
		ResourcePtr<ArtnetMeshFromFile> mMesh = nullptr;

		//property: Holds all universes that we want to use
		std::vector<ResourcePtr<nap::ArtNetController>> mArtnetControllers;
	};


	/**
	 * meshtoartnetcomponentInstance	
	 */
	class MeshToArtnetComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		MeshToArtnetComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize meshtoartnetcomponentInstance based on the meshtoartnetcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the meshtoartnetcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	Converts and sends the mesh attribute data over the network as art net messages
		 */
		virtual void update(double deltaTime) override;

	private:
		// All registered artnet controllers
		std::unordered_map<ArtNetController::Address, nap::ArtNetController*> mControllers;

		// Artnet mesh
		nap::ArtnetMeshFromFile* mMesh = nullptr;

		// Artnet array data
		std::vector<uint8> mArtnetData = std::vector<uint8>(4, 0);
	};
}
