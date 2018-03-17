#pragma once

#include <component.h>
#include <rtti/objectptr.h>
#include <artnetcontroller.h>
#include <nap/numeric.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <nap/resourceptr.h>

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
		ResourcePtr<ArtNetController> mController;
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

		/**
		 * Set the color using a set of floats
		 * @param color the new color in the 0-1 float range
		 */
		void setColor(const glm::vec3& color);

		/**
		 * Set the level of white
		 */
		void setWhite(float white);

		/**
		 * @return the color as a vec3 in the range of 0-1
		 */
		glm::vec3 getColor() const;

		/**
		 * @return the color as a 8bit unsigned int
		 * @param red the color red
		 * @param green the color green
		 * @param blue the color blue
		 * @param white the color white
		 */
		void getColor(uint8& red, uint8& green, uint8& blue, uint8& white);

		/**
		 * @return the value of white as a float
		 */
		float getWhite() const;

	private:
		ComponentInstancePtr<RenderableMeshComponent>	mRenderableMeshComponent = { this, &SelectColorComponent::mMesh };
		int												mStartChannel = 0;
		ArtNetController*								mArtnetController = nullptr;
		MaterialInstance*								mMaterial = nullptr;
		nap::uint8										mRed = 0;
		nap::uint8										mGreen = 0;
		nap::uint8										mBlue = 0;
		nap::uint8										mWhite = 0;
		bool											mDirty = true;
	};
}
