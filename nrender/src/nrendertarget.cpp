#include "nrendertarget.h"
#include "nglutils.h"


namespace opengl
{
	// Clears render target. If color is cleared, it is done using the clear color as set.
	void RenderTarget::clear(EClearFlags flags)
	{
		if ((flags & EClearFlags::Color) == EClearFlags::Color)
		{
			opengl::clearColor(mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a);
		}

		if ((flags & EClearFlags::Depth) == EClearFlags::Depth)
		{
			opengl::clearDepth();
		}

		if ((flags & EClearFlags::Stencil) == EClearFlags::Stencil)
		{
			opengl::clearStencil();
		}
	}
}