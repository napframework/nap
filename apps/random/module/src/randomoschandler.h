#pragma once

// Local Includes
#include "applycombinationcomponent.h"
#include "controlgroups.h"

// External Includes
#include <oscinputcomponent.h>
#include <nap/signalslot.h>
#include <componentptr.h>
#include <nap/resourceptr.h>

namespace nap
{
	class RandomOSCHandlerInstance;

	/**
	 *	RandomOSCHandler
	 */
	class NAPAPI RandomOSCHandler : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(RandomOSCHandler, RandomOSCHandlerInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<ApplyCombinationComponent> mCombinationComponent;	///< Property: 'CombinationComponent' link to the combination component
		ResourcePtr<ControlGroups> mControlGroups = nullptr;			///< Property: Pointer to the control groups
	};


	/**
	 * RandomOSCHandlerInstance	
	 */
	class NAPAPI RandomOSCHandlerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RandomOSCHandlerInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

		/**
		 * Initialize randomoschandlerInstance based on the randomoschandler resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the randomoschandlerInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update randomoschandlerInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		// Pointer to the run time Apply Combination Component Instance, set during de-serialization
		ComponentInstancePtr<ApplyCombinationComponent> mApplyCombinationComponent = { this, &RandomOSCHandler::mCombinationComponent };

	private:
		OSCInputComponentInstance* mOSCInputComponent = nullptr;				///< Handle to the component that receives the incoming osc messages
		ControlGroups* mControlGroups = nullptr;
		
		/**
		 * Called when the slot above is send a new message
		 * @param OSCEvent the new osc event
		 */
		void onEventReceived(const OSCEvent&);
		void updateBrightness(const OSCEvent& oscEvent, const std::vector<std::string>& args);
		void updateControlGroupBrightness(const OSCEvent& oscEvent, const std::vector<std::string>& args);

		// This map holds all the various callbacks based on id
		typedef void (RandomOSCHandlerInstance::*OscEventFunc)(const OSCEvent&, const std::vector<std::string>& args);
		std::unordered_map<std::string, OscEventFunc> mOscEventFuncs;

		/**
		 * Slot that is connected to the osc input component that receives new messages
		 */
		Slot<const OSCEvent&> eventReceivedSlot = { this, &RandomOSCHandlerInstance::onEventReceived };
	};
}
