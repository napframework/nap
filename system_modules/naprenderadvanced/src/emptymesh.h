/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <mesh.h>

namespace nap
{
    // Forward Declares
	class RenderService;

	/**
	 * Resource to generate a mesh instance without buffers. For simple rendering pipelines that rely on shader programs
	 * only. A common use case for this is rendering a full-screen color or texture using a single triangle. The
	 * `nap::RenderFaderComponent` is set up this way and serves as a good example.
	 *
	 * The following command invokes the vertex shader three times to render a single triangle. Logic for positioning the
	 * points is inside the vertex shader.
	 * 
	 * ~~~~~{.cpp}
	 * vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	 * ~~~~~
	 */
	class NAPAPI EmptyMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		EmptyMesh(Core& core);

		/**
		 * Initialize this dummy mesh
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual MeshInstance& getMeshInstance()	override				{ return *mMeshInstance; }

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual const MeshInstance& getMeshInstance() const	override	{ return *mMeshInstance; }

        EMemoryUsage			mUsage = EMemoryUsage::Static;		    ///< Property: 'Usage' GPU memory usage
        EDrawMode				mDrawMode = EDrawMode::Triangles;	    ///< Property: 'DrawMode' The draw mode that should be used to draw the shapes
        ECullMode				mCullMode = ECullMode::Back;		    ///< Property: 'CullMode' The triangle cull mode to use
        EPolygonMode			mPolygonMode = EPolygonMode::Fill;	    ///< Property: 'PolygonMode' The polygon mode to use, fill is always available and should be the default

	private:
		nap::RenderService* mRenderService = nullptr;					///< Handle to the render service
		std::unique_ptr<MeshInstance> mMeshInstance;					///< The mesh instance to construct
	};
}
