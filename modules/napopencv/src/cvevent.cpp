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


	CVFrameEvent::CVFrameEvent(const CVFrameEvent& other)
	{
		mFrames = other.mFrames;
	}


	CVFrameEvent& CVFrameEvent::operator=(const CVFrameEvent& other)
	{
		mFrames = other.mFrames;
		return *this;
	}


	void CVFrameEvent::addFrame(const CVFrame& frame)
	{
		mFrames.emplace_back(frame);
	}


	void CVFrameEvent::addFrame(CVFrame&& frame)
	{
		mFrames.emplace_back(std::move(frame));
	}


	const nap::CVFrame& CVFrameEvent::getFrame(int index) const
	{
		assert(index < mFrames.size());
		return mFrames[index];
	}


	const nap::CVFrame* CVFrameEvent::findFrame(const CVAdapter& adapter) const
	{
		// Check if adapter is managed by this capture device
		auto found_frame = std::find_if(mFrames.begin(), mFrames.end(), [&](const auto& it)
		{
			return it.getSource() == &adapter;
		});
		return found_frame != mFrames.end() ? &(*found_frame) : nullptr;
	}


	void CVFrameEvent::copyTo(CVFrameEvent& outEvent) const
	{
		outEvent.mFrames.resize(mFrames.size());
		for (auto i = 0; i < mFrames.size() ; i++)
		{
			mFrames[i].copyTo(outEvent[i]);
		}
	}


	nap::CVFrameEvent CVFrameEvent::clone() const
	{
		CVFrameEvent clone;
		clone.reserve(mFrames.size());
		for (auto& frame : mFrames)
		{
			clone.mFrames.emplace_back(frame.clone());
		}
		return clone;
	}
}