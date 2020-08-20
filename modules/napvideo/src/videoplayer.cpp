#include "videoplayer.h"

// nap::videoplayer run time class definition 
RTTI_BEGIN_CLASS(nap::VideoPlayer)
	// Put additional properties here
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	VideoPlayer::~videoplayer()			{ }


	bool VideoPlayer::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool VideoPlayer::start(utility::ErrorState& errorState)
	{
		return true;
	}


	void VideoPlayer::stop()
	{

	}
}