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

	private:
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;			///< The mesh instance to construct
		nap::RenderService* mRenderService = nullptr;					///< Handle to the render service
	};
}
