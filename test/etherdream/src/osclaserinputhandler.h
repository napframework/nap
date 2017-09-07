#pragma once

// Local Includes
#include "oscevent.h"

// External Includes
#include <nap/component.h>
#include <nap/signalslot.h>

#include <rotatecomponent.h>
#include <renderablemeshcomponent.h>
#include <oscinputcomponent.h>

namespace nap
{
	class OSCLaserInputHandlerInstance;

	/**
	* Interprets OSC events associated with laser shapes and actions
	*/
	class OSCLaserInputHandler : public Component
	{
		RTTI_ENABLE(Component)
	public:
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(OSCLaserInputHandlerInstance);
		}

		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


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
		nap::RenderableMeshComponentInstance* mMeshComponent = nullptr;
		nap::OSCInputComponentInstance* mInputComponent = nullptr;

		void updateColor(const OSCEvent& event);
		void updateRotate(const OSCEvent& event);

		NSLOT(mMessageReceivedSlot, const nap::OSCEvent&, handleMessageReceived)
	};
}
