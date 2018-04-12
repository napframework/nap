#pragma once

#include <component.h>
#include <rtti/objectptr.h>
#include <artnetcontroller.h>
#include <nap/numeric.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <nap/resourceptr.h>
#include <color.h>

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
		ComponentPtr<RenderableMeshComponent> mMesh;

		// property: channel to send data to
		int mStartChannel = 0;

		// property: rgb color as int
		RGBColorFloat mColor = { 0.0f, 0.0f, 0.0f };
		RColorFloat mWhite	= 0.0f;	
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
		void setColor(const RGBColorFloat& color);

		/**
		 * Set the level of white
		 */
		void setWhite(float white);

		/**
		 * @return the color as a vec3 in the range of 0-1
		 */
		RGBColorFloat getColor() const;

		/**
		 * @return the value of white as a float
		 */
		float getWhite() const;

		/**
		 *	Turn dirty on/off
		 */
		void setDirty()									{ mDirty = true; }

		RGBColorFloat									mColor;
		RColorFloat										mWhite;

	private:
		ComponentInstancePtr<RenderableMeshComponent>	mRenderableMeshComponent = { this, &SelectColorComponent::mMesh };
		MaterialInstance*								mMaterial = nullptr;
		bool											mDirty = true;
	};
}
