#pragma once

// Local Includes
#include "mesh.h"

// External Includes
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * This mesh builds a line structure based on the vertices of another mesh
	 * The line structure represents the normals of the reference mesh and can be drawn to screen. 
	 * The mesh itself carries 2 attributes, position and color, where
	 * the alpha value = 1 at the beginning of the normal and 0 at the end.
	 */
	class NAPAPI VisualizeNormalsMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
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
		 * @param push if the new positions should be pushed on to the GPU
		 */
		bool updateNormals(utility::ErrorState& error, bool push=true);

		// property: pointer to the IMesh that is used as a reference
		ResourcePtr<IMesh> mReferenceMesh = nullptr;							///< Property: 'ReferenceMesh' link to the mesh that is used as a reference
		
		// property: length of the normals
		float mNormalLength = 1.0f;												///< Property: 'Length' length of the normals

	protected:
		std::unique_ptr<MeshInstance> mMeshInstance;

		// Position Attribute data
		nap::Vec3VertexAttribute* mPositionAttr = nullptr;

		// 'Tip' vertex attribute data, 1.0 for top vertex of normal, 0.0 for bottom vertex of normal
		nap::FloatVertexAttribute* mTipAttr = nullptr;

		// UV attribute data
		std::vector<nap::Vec3VertexAttribute*> mUvAttrs;

		// Color attribute data
		std::vector<nap::Vec4VertexAttribute*> mColorAttrs;

		/**
		 * Sets up the mesh based on the reference mesh
		 * This call does not initialize the data on the GPU
		 * Use this function to create the mesh and initialize it in a derived class
		 * @param error contains the error when setup fails
		 * @return if setup succeeded or not
		 */
		bool setup(utility::ErrorState& error);
	};
}
