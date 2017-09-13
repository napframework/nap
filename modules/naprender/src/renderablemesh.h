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
	* Represent the coupling between a mesh and a material. Must be created through RenderableMeshComponentInstance.
	*/
	class NAPAPI RenderableMesh final
	{
	public:
		RenderableMesh() = default;

		/**
		* @return whether the material and mesh form a valid combination. It is valid when the vertex attributes
		* of the mesh match with the vertex attributes of the shader that is applied on the material.
		*/
		bool isValid() const													{ return mVAOHandle.isValid(); }

		/**
		* @return The IMesh object that was used to create this object.
		*/
		IMesh& getMesh()														{ return *mMesh; }

		/**
		* @return The IMesh object that was used to create this object.
		*/
		const IMesh& getMesh() const											{ return *mMesh; }

		/**
		* @return The MaterialInstance object that was used to create this object.
		*/
		MaterialInstance& getMaterialInstance()									{ return *mMaterialInstance; }

		/**
		* @return The MaterialInstance object that was used to create this object.
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
