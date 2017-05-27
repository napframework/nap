#pragma once

#include <nap/plug.h>

class Wire : public nap::Object
{
public:
	Wire(nap::OutputPlugBase& srcPlug, nap::InputPlugBase& dstPlug)
		: nap::Object(), mSrcPlug(srcPlug), mDstPlug(dstPlug)
	{
	}

	nap::OutputPlugBase& getSrcPlug() const { return mSrcPlug; }
	nap::InputPlugBase& getDstPlug() const { return mDstPlug; }


private:
	nap::OutputPlugBase& mSrcPlug;
	nap::InputPlugBase& mDstPlug;
};
