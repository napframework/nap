#pragma once

// Local includes
#include "mesh.h"
#include "materialinstance.h"
#include "nap/signalslot.h"

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
		bool isValid() const													{ return mMesh != nullptr; }

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

		const std::vector<VkBuffer>& getVertexBuffers() const { return mVertexBuffers; }
		const std::vector<VkDeviceSize>& getVertexBufferOffsets() const { return mVertexBufferOffsets; }

	protected:
		/**
		 * Constructor
		 * @param mesh the mesh that is rendered
		 * @param materialInstance the material the mesh is rendered with
		 * @param vaoHandle issued by the render service based on mesh / material combination
		 */
		RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance);

	private:
		MaterialInstance*			mMaterialInstance = nullptr;	///< Material instance
		IMesh*						mMesh = nullptr;				///< Mesh
		std::vector<VkBuffer>		mVertexBuffers;
		std::vector<VkDeviceSize>	mVertexBufferOffsets;
	};

}
