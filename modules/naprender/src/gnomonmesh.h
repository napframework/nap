/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>
#include <renderablemesh.h>

namespace nap
{
	/**
	 * Simple 3D Gnomon mesh, used for orientation. 
	 * x-axis = red, y-axis = green, z-axis = blue.
	 * By default the length of every axis is 1 unit.
	 */
	class NAPAPI GnomonMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		// Constructor
		GnomonMesh(Core& core);

		/**
		* Creates the mesh.
		* @param errorState contains the error message when initialization fails.
		* @return if initialization succeeded.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the mesh instance
		 */
		virtual MeshInstance& getMeshInstance()							{ return *mMeshInstance; }

		/**
		 * @return the mesh instance
		 */
		virtual const MeshInstance& getMeshInstance() const	override	{ return *mMeshInstance; }

		float mSize = 1.0f;												///< Property: 'Size' axis length
		glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };						///< Property: 'Position' position in object space

	private:
		std::unique_ptr<nap::MeshInstance> mMeshInstance = nullptr;
		nap::RenderService* mRenderService = nullptr;
	};
}
