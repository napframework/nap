/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "mesh.h"

// External Includes
#include <nap/resourceptr.h>

namespace nap
{
	// Forward Declares
	class Core;
	class RenderService;

	/**
	 * This mesh builds a line structure based on the vertices of another mesh
	 * The line structure represents the normals of the reference mesh and can be drawn to screen. 
	 * The mesh itself carries multiple attributes of which 'position' and 'típ' are default.
	 * Use the tip vertex attribute to identify the bottom / top part of the vertex.
	 * the tip value = 1 at the beginning of the normal line and 0 at the end.
	 * This resource also copies the color, normal and uv channels as attributes to both points of the normal line.
	 * The normal is scaled based on the normal length property and applied to both vertices of the normal line
	 * This operator needs a reference mesh to initialize itself, defined by the 'ReferenceMesh' property.
	 * It is possible to switch the reference mesh at runtime, although discouraged.
	 * When you switch the reference mesh at runtime make sure the new reference mesh has the same number of attributes!
	 */
	class NAPAPI VisualizeNormalsMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		VisualizeNormalsMesh(Core& core);

		/**
		 *	Creates the normals from the reference mesh
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	@return the mesh that holds the normals
		 */
		virtual MeshInstance& getMeshInstance() override					{ return *mMeshInstance; }

		/**
		 *	@return the mesh that holds the normals
		 */
		virtual const MeshInstance& getMeshInstance() const override		{ return *mMeshInstance; }

		/**
		* Updates the normals based on the data in the reference mesh
		* @param error contains the error when normals could not be pushed to the GPU.
		* @param push if the new positions should be pushed on to the GPU.
		*/
		bool calculateNormals(utility::ErrorState& error, bool push = true);

		ResourcePtr<IMesh> mReferenceMesh = nullptr;							///< Property: 'ReferenceMesh' link to the mesh that is used as a reference, can be null (ie: nothing)
		float mNormalLength = 1.0f;												///< Property: 'Length' length of the normals
		EMemoryUsage	mUsage = EMemoryUsage::DynamicWrite;					///< Property: 'Usage' If the normals are created once or frequently updated.

		/**
		* Set the mesh that is used as a reference to build the normals from. Called on init().
		* Note that it is not recommended setting this at runtime, if you do, make sure that the mesh
		* you set has the same number of vertex attributes as the original reference mesh!
		* This call automatically calls setup, where storage for the CPU side of the buffer is calculated
		* Call calculateNormals() to actually calculate the normals based on the reference mesh and push the data to the GPU
		*/
		bool setReferenceMesh(IMesh& mesh, nap::utility::ErrorState& error);

	protected:
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;

		// Position Attribute data
		nap::Vec3VertexAttribute* mPositionAttr = nullptr;

		// 'Tip' vertex attribute data, 1.0 for top vertex of normal, 0.0 for bottom vertex of normal
		nap::FloatVertexAttribute* mTipAttr = nullptr;

		// Normal Attribute data
		nap::Vec3VertexAttribute* mNormalsAttr = nullptr;

		// UV attribute data
		std::vector<nap::Vec3VertexAttribute*> mUvAttrs;

		// Color attribute data
		std::vector<nap::Vec4VertexAttribute*> mColorAttrs;

		// The current reference mesh
		nap::IMesh* mCurrentReferenceMesh = nullptr;

		// The renderer
		nap::RenderService* mRenderService = nullptr;

		/**
		 * Creates the mesh instance and single shape associated with that mesh instance
		 * Called during init but can be called in derived classes to create the mesh instance
		 */
		bool createMeshInstance(utility::ErrorState& error);

	private:
		/**
		 * Creates the normal mesh attributes CPU side based on the currently available reference mesh
		 * This call does not initialize the data on the GPU
		 * @param error contains the error when setup fails
		 * @return if setup succeeded or not
		 */
		bool setup(utility::ErrorState& error);
	};
}
