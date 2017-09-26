#include "nrendertarget.h"
#include "nglutils.h"


namespace opengl
{
	// Clears render target. If color is cleared, it is done using the clear color as set.
	void RenderTarget::clear(EClearFlags flags)
	{
		if ((flags & EClearFlags::COLOR) == EClearFlags::COLOR)
		{
			opengl::clearColor(mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a);
		}

		if ((flags & EClearFlags::DEPTH) == EClearFlags::DEPTH)
		{
			opengl::clearDepth();
		}

		if ((flags & EClearFlags::STENCIL) == EClearFlags::STENCIL)
		{
			opengl::clearStencil();
		}
	}
}