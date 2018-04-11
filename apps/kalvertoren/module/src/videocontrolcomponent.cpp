#include "videocontrolcomponent.h"

// External Includes
#include <entity.h>
#include <video.h>
#include <mathutils.h>

// nap::controlvideocomponent run time class definition 
RTTI_BEGIN_CLASS(nap::VideoControlComponent)
	RTTI_PROPERTY("Player",		&nap::VideoControlComponent::mVideoPlayer,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FadeTime",	&nap::VideoControlComponent::mFadeTime,		nap::rtti::EPropertyMetaData::Default)
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
		// Store video player
		mVideoPlayer = getComponent<VideoControlComponent>()->mVideoPlayer.get();
		mVideoPlayer->mLoop = false;
		
		// Store fade time
		mFadeTime = getComponent<VideoControlComponent>()->mFadeTime;
		return true;
	}


	void VideoControlComponentInstance::update(double deltaTime)
	{

	}


	void VideoControlComponentInstance::render()
	{

	}


	float VideoControlComponentInstance::getIntensity()
	{
		if (!mVideoPlayer->isPlaying())
			return 0.0f;

		double current_time = mVideoPlayer->getCurrentTime();
		double video_length = mVideoPlayer->getDuration();
		double fade_in = math::fit<double>(current_time, 0.0, static_cast<double>(mFadeTime), 0.0, 1.0);
		double fade_ou = math::fit<double>(current_time, video_length - static_cast<double>(mFadeTime), video_length, 1.0, 0.0);
		return static_cast<float>(pow(fade_in * fade_ou, 2.0));
	}


	void VideoControlComponentInstance::play(bool value)
	{
		if (mVideoPlayer->isPlaying())
		{
			mVideoPlayer->stop(true);
		}

		if (value)
		{
			mVideoPlayer->play();
		}
	}
}