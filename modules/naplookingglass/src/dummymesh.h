/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#pragma once

 // External Includes
#include <renderablemesh.h>

namespace nap
{
	class RenderService;
	class Core;

	/**
	 * Dummy mesh to generate a mesh instance without buffers. This can be used to generate a simple
	 * low-overhead fullscreen rendering pipeline for e.g. post-processing.
	 *
	 * Check out this article for an explanation:
	 * https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
	 */
	class DummyMesh : public IMesh
	{
	public:
		DummyMesh(Core& core);

		/**
		 * Initialize this dummy mesh
		 */
		virtual bool init(utility::ErrorState& errorState) override;
		
		/**
		 * @return MeshInstance as created during init().
		 */
		virtual MeshInstance& getMeshInstance()	override { return *mMeshInstance; }

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual const MeshInstance& getMeshInstance() const	override { return *mMeshInstance; }

	private:
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;			///< The mesh instance to construct
		RenderService* mRenderService = nullptr;						///< Handle to the render service
	};
}
