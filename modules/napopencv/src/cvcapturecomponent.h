/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	 * Receives frame updates from a nap::CVCaptureDevice on the main application thread.
	 * The actual capture operation is handled by the nap::CVCaptureDevice on a background thread.
	 * Register to the 'frameReceived' signal of the nap::CVCaptureComponentInstance to 
	 * receive frame updates on the main processing thread.
	 */
	class NAPAPI CVCaptureComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CVCaptureComponent, CVCaptureComponentInstance)
	public:
		ResourcePtr<CVCaptureDevice> mDevice;		///< Property: 'Device' the device this component receives frames from
	};


	/**
	 * Receives frame updates from a nap::CVCaptureDevice on the main application thread.
	 * The actual capture operation is handled by the nap::CVCaptureDevice on a background thread.
	 * Register to the 'frameReceived' signal to receive frame updates on the main processing thread.
	 *
	 * Note that the frame that is forwarded to potential listeners is a reference.
	 * Do not modify the content of the frame! Changes will be forwarded to other listeners.
	 * To get an actual copy of the content of the frame call FrameEvent::clone().
	 * Do this before performing any operation on the frame itself.
	 */
	class NAPAPI CVCaptureComponentInstance : public ComponentInstance
	{
		friend class CVService;
		RTTI_ENABLE(ComponentInstance)
	public:
		/**
		 * @param entity the entity this component belongs to.
		 * @param resource the resource this instance was created from.
		 */
		CVCaptureComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		virtual ~CVCaptureComponentInstance();

		/**
		 * Initializes the capture component instance.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Returns the signal that is called when a new frame is received.
		 * @return the frame received signal.
		 */
		Signal<const CVFrameEvent&>* getFrameReceived() { return &frameReceived; }

		/**
		 * @return the capture device this component receives frames from
		 */
		const CVCaptureDevice& getDevice() const;

		/**
		 * @return the capture device this component receives frames from
		 */
		CVCaptureDevice& getDevice();

		// The signal that is emitted when a new frame is received
		Signal<const CVFrameEvent&>			frameReceived;		///< Triggered when the component receives a new opencv frame

	private:
		/**
		 * This is called by the service when a new frame is received.
		 * @param frameEvent the frame that will be forwarded by this component
		 */
		void trigger(const nap::CVFrameEvent& frameEvent);

		CVCaptureDevice* mDevice;
		CVService* mService = nullptr;
	};
}
