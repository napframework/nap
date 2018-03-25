#pragma once

// Local Includes
#include "selectcolorcomponent.h"

// External Includes
#include <component.h>
#include <artnetcontroller.h>

namespace nap
{
	class SendColorComponentInstance;

	/**
	 *	sendcolorcomponent
	 */
	class SendColorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SendColorComponent, SendColorComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Property: All the artnet controllers
		std::vector<nap::ResourcePtr<ArtNetController>> mControllers;

		// Property: Span of the colors
		int mSpan = 1;
	};


	/**
	 * sendcolorcomponentInstance	
	 */
	class SendColorComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SendColorComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize sendcolorcomponentInstance based on the sendcolorcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the sendcolorcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update sendcolorcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Set span per color
		 */
		void setSpan(int value);

		/**
		 *	Span per color
		 */
		int mSpan = 1;

	private:
		// Holds all the controllers
		std::vector<ArtNetController*> mControllers;
		
		std::vector<SelectColorComponentInstance*> mColorComps;
		std::vector<RGBColor8> mColors;
		std::vector<RColor8> mWhites;

		// Update colors
		void convertColors();
	};
}
