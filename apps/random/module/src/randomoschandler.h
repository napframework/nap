#pragma once

// Local Includes
#include "lightingmodecomponent.h"
#include "applycombinationcomponent.h"
#include "updatematerialcomponent.h"
#include "selectvideocomponent.h"
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

		ComponentPtr<LightingModeComponent> mLightingModeComponent;			///< Property: 'LightingModeComponent' link to the lighting mode component
		ComponentPtr<ApplyCombinationComponent> mCombinationComponent;		///< Property: 'CombinationComponent' link to the combination component
		ComponentPtr<UpdateMaterialComponent> mUpdateMaterialComponent;		///< Property: 'UpdateMaterialComponent' link to the update material component
		ComponentPtr<SelectVideoComponent> mSelectVideoComponent;			///< Property: 'SelectVideoComponent' link to the select video component
		ResourcePtr<ControlGroups> mControlGroups = nullptr;				///< Property: Pointer to the control groups
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

		// Pointers to the run time Component Instances, set during de-serialization
		ComponentInstancePtr<LightingModeComponent> mLightingModeComponent = { this, &RandomOSCHandler::mLightingModeComponent };
		ComponentInstancePtr<ApplyCombinationComponent> mApplyCombinationComponent = { this, &RandomOSCHandler::mCombinationComponent };
		ComponentInstancePtr<UpdateMaterialComponent> mUpdateMaterialComponent = { this, &RandomOSCHandler::mUpdateMaterialComponent };
		ComponentInstancePtr<SelectVideoComponent> mSelectVideoComponent = { this, &RandomOSCHandler::mSelectVideoComponent };

	private:
		OSCInputComponentInstance* mOSCInputComponent = nullptr;				///< Handle to the component that receives the incoming osc messages
		ControlGroups* mControlGroups = nullptr;
		
		/**
		 * Called when the slot above is send a new message
		 * @param OSCEvent the new osc event
		 */
		void onEventReceived(const OSCEvent&);
		void updateLightingMode(const OSCEvent& oscEvent, const std::vector<std::string>& args);
		void updateBrightness(const OSCEvent& oscEvent, const std::vector<std::string>& args);
		void updateControlGroupBrightness(const OSCEvent& oscEvent, const std::vector<std::string>& args);
		void updateStaticTemperature(const OSCEvent& oscEvent, const std::vector<std::string>& args);
		void updateVideoIndex(const OSCEvent& oscEvent, const std::vector<std::string>& args);
		void updatePartyGlitchIntensity(const OSCEvent& oscEvent, const std::vector<std::string>& args);

		// This map holds all the various callbacks based on id
		typedef void (RandomOSCHandlerInstance::*OscEventFunc)(const OSCEvent&, const std::vector<std::string>& args);
		std::unordered_map<std::string, OscEventFunc> mOscEventFuncs;

		/**
		 * Slot that is connected to the osc input component that receives new messages
		 */
		Slot<const OSCEvent&> eventReceivedSlot = { this, &RandomOSCHandlerInstance::onEventReceived };
	};
}
