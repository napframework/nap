/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "mesh.h"

// External includes
#include <box.h>

namespace nap
{
	class RenderService;
	class CameraComponentInstance;

	/**
	 * Predefined box frame line mesh for objects to aid in visual debugging (e.g. `nap::RenderFrustumComponent`).
	 * Consists of a single position vertex attribute and is rendered as a line only using EPolygonMode::Line.
	 */
	class NAPAPI BoxFrameMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		BoxFrameMesh(Core& core);

		/**
		 * Sets up and initializes the box as a mesh based on the provided parameters.
		 * @param errorState contains the error message if the mesh could not be created.
		 * @return if the mesh was successfully created and initialized.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Creates and prepares the mesh but doesn't initialize it.
		 * Call this when you want to prepare a box without creating the GPU representation.
		 * You have to manually call init() on the mesh instance afterwards.
		 */
		bool setup(const math::Box& box, utility::ErrorState& errorState);

		/**
		 * @return the mesh instance that can be rendered to screen
		 */
		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }

		/**
		 * @return the mesh instance that can be rendered to screen
		 */
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

		/**
		 * @return the unit line box from (-0.5, -0.5, -0.5) to (0.5, 0.5, 0.5)
		 */
		const std::vector<glm::vec3>& getUnitLineBox();

		/**
		 * @return the normalized line box from (-1, -1, -1) to (1, 1, 1)
		 */
		const std::vector<glm::vec3>& getNormalizedLineBox();

		EMemoryUsage mUsage = EMemoryUsage::Static;			///< Property: 'Usage' If the mesh is uploaded once or frequently updated
		bool mUnit = false;									///< Property: 'Extent' Extent of the box (0.5)

	protected:
		/**
		 * Creates and prepares the mesh but doesn't initialize it.
		 * Call this when you want to prepare a box without creating the GPU representation.
		 * You have to manually call init() on the mesh instance afterwards.
		 */
		void setup();

		RenderService* mRenderService;
		std::unique_ptr<MeshInstance> mMeshInstance;

	private:
		bool mIsSetup = false;
	};
}
