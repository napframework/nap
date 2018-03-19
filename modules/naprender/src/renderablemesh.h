#pragma once

// Local includes
#include "mesh.h"
#include "material.h"
#include "vao.h"

namespace nap
{
	// Forward Declares
	class RenderableMeshComponentInstance;

	/**
	 * Represents the coupling between a mesh and a material. This object can only be created by a RenderableMeshComponentInstance
	 * This object manages a vertex array object handle that is issued by the render service. 
	 */
	class NAPAPI RenderableMesh final
	{
	public:
		RenderableMesh() = default;

		/**
		* @return whether the material and mesh form a valid combination. The combination is valid when the vertex attributes
		* of a mesh match the vertex attributes of a shader.
		*/
		bool isValid() const													{ return mVAOHandle.isValid(); }

		/**
		* @return The mesh object used to create this object.
		*/
		IMesh& getMesh()														{ return *mMesh; }

		/**
		* @return The mesh object used to create this object.
		*/
		const IMesh& getMesh() const											{ return *mMesh; }

		/**
		* @return The material instance object used to create this object.
		*/
		MaterialInstance& getMaterialInstance()									{ return *mMaterialInstance; }

		/**
		* @return The material instance object used to create this object.
		*/
		const MaterialInstance& getMaterialInstance() const						{ return *mMaterialInstance; }

	private:
		friend class RenderableMeshComponentInstance;

		/**
		 * Constructor.
		 */
		RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, const VAOHandle& vaoHandle);

		MaterialInstance*	mMaterialInstance = nullptr;	///< Material instance
		IMesh*				mMesh = nullptr;				///< Mesh
		VAOHandle			mVAOHandle;						///< Vertex Array Object handle, acquired from the RenderService
	};

}
