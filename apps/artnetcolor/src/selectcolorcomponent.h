#pragma once

#include <nap/component.h>
#include <nap/objectptr.h>
#include <artnetcontroller.h>
#include <nap/configure.h>
#include <nap/componentptr.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	class SelectColorComponentInstance;

	/**
	 *	selectcolorcomponent
	 */
	class SelectColorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectColorComponent, SelectColorComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// property: link to the artnet controller
		ObjectPtr<ArtNetController> mController;
		ComponentPtr<RenderableMeshComponent> mMesh;

		// property: channel to send data to
		int mStartChannel = 0;

		// property: rgb color as int
		int mRed	= 0;	// 0 - 255
		int mGreen	= 0;	// 0 - 255
		int mBlue	= 0;	// 0 - 255
		int mWhite	= 0;	// 0 - 255
	};


	/**
	 * selectcolorcomponentInstance	
	 */
	class SelectColorComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SelectColorComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize selectcolorcomponentInstance based on the selectcolorcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the selectcolorcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	Send the color values to the dmx output
		 */
		virtual void update(double deltaTime) override;

	private:
		ComponentInstancePtr<RenderableMeshComponent>	mRenderableMeshComponent = { this, &SelectColorComponent::mMesh };
		int												mStartChannel = 0;
		ArtNetController*								mArtnetController = nullptr;
		MaterialInstance*								mMaterial = nullptr;
		nap::uint8										mRed = 0;
		nap::uint8										mGreen = 0;
		nap::uint8										mBlue = 0;
		nap::uint8										mWhite = 0;
	};
}
