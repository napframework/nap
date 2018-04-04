#include "videocontrolcomponent.h"

// External Includes
#include <entity.h>

// nap::controlvideocomponent run time class definition 
RTTI_BEGIN_CLASS(nap::VideoControlComponent)
	RTTI_PROPERTY("Player", &nap::VideoControlComponent::mVideoPlayer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::controlvideocomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VideoControlComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void VideoControlComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool VideoControlComponentInstance::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void VideoControlComponentInstance::update(double deltaTime)
	{

	}


	void VideoControlComponentInstance::render()
	{

	}

}