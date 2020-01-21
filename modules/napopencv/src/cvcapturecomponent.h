#pragma once

// Local includes
#include "cvcapturedevice.h"

// External Includes
#include <component.h>
#include <unordered_set>

namespace nap
{
	class CVCaptureComponentInstance;
	class CVService;

	/**
	 *	cvcapturecomponent
	 */
	class NAPAPI CVCaptureComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CVCaptureComponent, CVCaptureComponentInstance)
	public:
		std::vector<ResourcePtr<CVCaptureDevice>> mDevices;		///< Property: 'Devices' the devices this component receives frames from, 0 means all devices
	};


	/**
	 * cvcapturecomponentInstance	
	 */
	class NAPAPI CVCaptureComponentInstance : public ComponentInstance
	{
		friend class CVService;
		RTTI_ENABLE(ComponentInstance)
	public:
		CVCaptureComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		virtual ~CVCaptureComponentInstance();

		/**
		 * Initialize cvcapturecomponentInstance based on the cvcapturecomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the cvcapturecomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update cvcapturecomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		Signal<const CVFrameEvent&>			frameReceived;		///< Triggered when the component receives a new opencv frame

		/**
		 * Returns the signal that is called when a new frame is received.
		 * @return the frame received signal.
		 */
		Signal<const CVFrameEvent&>* getFrameReceived() { return &frameReceived; }

	private:
		/**
		 * This is called by the service when a new frame is received.
		 * @param frameEvent the frame that will be forwarded by this component
		 */
		void trigger(const nap::CVFrameEvent& frameEvent);

		std::unordered_set<CVCaptureDevice*> mDevices;
		CVService* mService = nullptr;
	};
}
