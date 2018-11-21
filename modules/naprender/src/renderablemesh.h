#pragma once

// Local includes
#include "mesh.h"
#include "material.h"
#include "vao.h"

namespace nap
{
	// Forward Declares
	class RenderService;

	/**
	 * Represents the coupling between a mesh and a material that can be rendered to screen.
	 * Use the vertex array object handle to bind your mesh before rendering.
	 * The vertex array object handle is given by the render service on construction.
	 * Call RenderService.createRenderableMesh() from your own renderable component to create a renderable mesh on initialization.
	 */
	class NAPAPI RenderableMesh final
	{
		friend class RenderService;
	public:
		/**
		 * Default constructor, by default this represents an invalid mesh / material combination.
		 * Can be used to declare a member of this type that is invalid on initialization.
		 */
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

		/**
		 * Binds the managed vertex array object. Call this before rendering your vertex buffers.
		 * Only call this when the VAO is considered valid!
		 */
		void bind();
		
		/**
		 * Unbinds the managed vertex array object. Call this after rendering your vertex buffers.
		 * Only call this when the VAO is considered valid!
		 */
		void unbind();

	protected:
		/**
		 * Constructor
		 * @param mesh the mesh that is rendered
		 * @param materialInstance the material the mesh is rendered with
		 * @param vaoHandle, issued by the render service based on mesh / material combination
		 */
		RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, const VAOHandle& vaoHandle);

	private:
		MaterialInstance*	mMaterialInstance = nullptr;	///< Material instance
		IMesh*				mMesh = nullptr;				///< Mesh
		VAOHandle			mVAOHandle;						///< Vertex Array Object handle, acquired from the RenderService
	};

}
