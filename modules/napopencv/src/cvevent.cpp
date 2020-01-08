#include "cvevent.h"

RTTI_BEGIN_CLASS(nap::CVFrameEvent)
RTTI_END_CLASS

namespace nap
{
	CVFrameEvent::CVFrameEvent(const std::vector<CVFrame>& frames)
	{
		mFrames = frames;
	}


	CVFrameEvent::CVFrameEvent(const std::vector<CVFrame>&& frames)
	{
		mFrames = std::move(frames);
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


	void CVFrameEvent::copyTo(CVFrameEvent& outEvent) const
	{
		outEvent.mFrames.clear();
		outEvent.mFrames.reserve(mFrames.size());
		for (auto& frame : mFrames)
		{
			outEvent.addFrame(CVFrame());
			CVFrame& last_frame = outEvent.mFrames.back();
			frame.copyTo(last_frame);
		}
	}
}