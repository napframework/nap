#pragma once

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <mutex>
#include <thread>
#include <future>

// Local Includes
#include "oscservice.h"
#include "parameter.h"
#include "oscsender.h"
#include "oscreceiver.h"
#include "parameternumeric.h"
#include "oscinputcomponent.h"

namespace nap
{
	// forward declares
	class TouchOscComponentInstance;

	class NAPAPI TouchOscParameterLink : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// deconstructor
		virtual ~TouchOscParameterLink() override;
	public:
		// properties
		std::string					mAdress;
		ResourcePtr<ParameterFloat>	mParameter;
	};

	class NAPAPI TouchOscComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(TouchOscComponent, TouchOscComponentInstance)
	public:
		// properties
		std::vector<ResourcePtr<TouchOscParameterLink>>		mLinks;
		ResourcePtr<OSCSender>								mOSCSender;
		ComponentPtr<OSCInputComponent>						mOSCInputComponent;
	};

	class NAPAPI TouchOscComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		TouchOscComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

		virtual ~TouchOscComponentInstance();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;
	protected:
		void onMessageReceived(const OSCEvent &event);
	private:
		std::vector<TouchOscParameterLink*>				mLinks;
		OSCSender*										mOSCSender;
		ComponentInstancePtr<OSCInputComponent>			mOSCInput = initComponentInstancePtr(this, &TouchOscComponent::mOSCInputComponent);
		bool											mEnableSend = true;
	};
}