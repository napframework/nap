#pragma once

#include <component.h>
#include <apicomponent.h>
#include <nap/signalslot.h>

namespace nap
{
	class EmographyAPIComponentInstance;

	/**
	 *	APIHandleComponent
	 */
	class NAPAPI EmographyAPIComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(EmographyAPIComponent, EmographyAPIComponentInstance)
	public:

		/**
		* This component depends on an api component.
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * apihandlecomponentInstance	
	 */
	class NAPAPI EmographyAPIComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		EmographyAPIComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize apihandlecomponentInstance based on the apihandlecomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the apihandlecomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update apihandlecomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		APIComponentInstance* mComponentInstance = nullptr;

		// All callbacks
		void updateView(const nap::APIEvent& apiEvent);
		nap::Slot<const nap::APIEvent&> mUpateViewSlot = { this, &EmographyAPIComponentInstance::updateView };
	};
}
