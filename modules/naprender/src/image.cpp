#include "image.h"
#include "nap/core.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Image)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	Image::Image(Core& core) :
		Texture2D(core)
	{
	}


	void Image::update()
	{
		assert(!mBitmap.empty());
		update(mBitmap.getData(), mBitmap.mSurfaceDescriptor);
	}


	void Image::asyncGetData()
	{
		asyncGetData(mBitmap);
	}
}
