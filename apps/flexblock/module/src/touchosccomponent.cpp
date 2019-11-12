// Local Includes
#include "touchosccomponent.h"

RTTI_BEGIN_CLASS(nap::TouchOscParameterLink)
// Put additional properties here
	RTTI_PROPERTY("Adress",		&nap::TouchOscParameterLink::mAdress,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Parameter",	&nap::TouchOscParameterLink::mParameter,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::TouchOscComponent run time class definition 
RTTI_BEGIN_CLASS(nap::TouchOscComponent)
// Put additional properties here
	RTTI_PROPERTY("Links",			&nap::TouchOscComponent::mLinks,				nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("OSCSender",		&nap::TouchOscComponent::mOSCSender,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OSCInput",		&nap::TouchOscComponent::mOSCInputComponent,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TouchOscComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)

RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	TouchOscParameterLink::~TouchOscParameterLink() {}

	TouchOscComponentInstance::~TouchOscComponentInstance()
	{ 
	}

	bool TouchOscComponentInstance::init(utility::ErrorState& errorState)
	{
		//
		TouchOscComponent* resource = getComponent<TouchOscComponent>();
		mOSCSender = resource->mOSCSender.get();

		//
		for (auto& link : resource->mLinks)
		{
			mLinks.emplace_back(link.get());
		}

		//
		mOSCInput->messageReceived.connect(this, &TouchOscComponentInstance::onMessageReceived);

		//
		for (auto& link : mLinks)
		{
			link->mParameter->valueChanged.connect([this, link](float arg)
			{
				if (this->mEnableSend)
				{
					OSCEvent oscEvent(link->mAdress);
					oscEvent.addValue<float>(arg);
					this->mOSCSender->send(oscEvent);
				}
			});
		}

		return true;
	}


	void TouchOscComponentInstance::onMessageReceived(const OSCEvent & event)
	{
		//printf("received [%s] and value [%f]\n", event.getAddress().c_str(), event.getArgument(0)->asFloat());
		for (const auto* link : mLinks)
		{
			if (link->mAdress == event.getAddress())
			{
				mEnableSend = false;
				link->mParameter->setValue(event.getArgument(0)->asFloat());
				mEnableSend = true;
			}
		}
	}
}