#pragma once

// Local Includes
#include "oscevent.h"
#include "lineselectioncomponent.h"
#include "lineblendcomponent.h"
#include "linecolorcomponent.h"
#include "laseroutputcomponent.h"
#include "linemodulationcomponent.h"
#include "linenoisecomponent.h"

// External Includes
#include <nap/component.h>
#include <nap/signalslot.h>
#include <nap/componentptr.h>

#include <rotatecomponent.h>
#include <renderablemeshcomponent.h>
#include <oscinputcomponent.h>
#include <transformcomponent.h>


namespace nap
{
	class OSCLaserInputHandlerInstance;

	/**
	* Interprets OSC events associated with laser shapes and actions
	*/
	class OSCLaserInputHandler : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(OSCLaserInputHandler, OSCLaserInputHandlerInstance)
	public:
		// property: Link to selection component one
		ComponentPtr<LineSelectionComponent> mSelectionComponentOne = nullptr;

		// property: Link to selection component two
		ComponentPtr<LineSelectionComponent> mSelectionComponentTwo = nullptr;

		// property: Link to laser output component
		ComponentPtr<LaserOutputComponent> mLaserOutputComponent = nullptr;

		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 *	Helper component that translates received OSC events in to specific app actions
	 */
	class OSCLaserInputHandlerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		OSCLaserInputHandlerInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

		// Destructor
		virtual ~OSCLaserInputHandlerInstance();

		/**
		 *	Retrieve necessary components for osc input translation
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

	private:
		void handleMessageReceived(const nap::OSCEvent& oscEvent);

		nap::RotateComponentInstance* mRotateComponent = nullptr;
		nap::OSCInputComponentInstance* mInputComponent = nullptr;
		nap::LineBlendComponentInstance* mBlendComponent = nullptr;
		nap::TransformComponentInstance* mTransformComponent = nullptr;
		nap::LineNoiseComponentInstance* mNoiseComponent = nullptr;

		void updateColor(const OSCEvent& event, int position);
		void updateRotate(const OSCEvent& event);
		void resetRotate(const OSCEvent& event);
		void setIndex(const OSCEvent& event, int index);
		void setBlend(const OSCEvent& event, int index);
		void setScale(const OSCEvent& event);
		void setPosition(const OSCEvent& event);
		void setModulation(const OSCEvent& event, int index);
		void setNoise(const OSCEvent& event, int index);
		void setColorSync(const OSCEvent& event);

		NSLOT(mMessageReceivedSlot, const nap::OSCEvent&, handleMessageReceived)

		LineSelectionComponentInstance* mSelectorOne = nullptr;		// First line selection component
		LineSelectionComponentInstance* mSelectorTwo = nullptr;		// Second line selection component
		LaserOutputComponentInstance* mLaserOutput = nullptr;		// Laser output component
		LineColorComponentInstance* mColorComponent = nullptr;		// Laser line color component
		LineModulationComponentInstance* mModulationComponent = nullptr;	// Laser modulation component

		float mInitialScale = 1.0f;									// Holds the initial scale of the laser spline entity
	};
}
