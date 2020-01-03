#include "cvevent.h"

RTTI_BEGIN_CLASS(nap::CVFrameEvent)
RTTI_END_CLASS

namespace nap
{
	CVFrameEvent::CVFrameEvent(const CVFrame& frame)
	{
		mFrames.emplace_back(frame);
	}


	CVFrameEvent::CVFrameEvent(CVFrame&& frame)
	{
		mFrames.emplace_back(std::move(frame));
	}


	CVFrameEvent::CVFrameEvent(int count)
	{
		mFrames.resize(count);
	}


	void CVFrameEvent::addFrame(const CVFrame& frame)
	{
		mFrames.emplace_back(frame);
	}


	void CVFrameEvent::addFrame(CVFrame&& frame)
	{
		mFrames.emplace_back(std::move(frame));
	}


	const nap::CVFrame& CVFrameEvent::getFrame(int index)
	{
		assert(index < mFrames.size());
		return mFrames[index];
	}

}