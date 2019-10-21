#pragma once

// Local Includes
#include "nvertexarrayobject.h"
#include "nglutils.h"

// External Includes
#include <memory>

namespace opengl
{
	/**
	 * Defines the GPU data of a polygonal mesh.
	 */
	class GPUMesh
	{
	public:
		GPUMesh() = default;

		// Default destructor
		virtual ~GPUMesh() = default;

		GPUMesh(const GPUMesh& other) = delete;
		GPUMesh& operator=(const GPUMesh& other) = delete;

		/**
		 * Adds a vertex attribute stream to the mesh
		 * @param id name of the vertex attribute
		 * @param type vertex attribute type, for example: GL_FLOAT
		 * @param numComponents number of component per element (for instance, 3 for vector with 3 floats)
		 * @param usage attribute usage definition, for example: GL_STATIC_DRAW
		 */
		void addVertexAttribute(const std::string& id, GLenum type, unsigned int numComponents, GLenum usage);

		/**
		 * @param id name of the vertex attribute
		 * @return reference to the attribute buffer if found, otherwise nullptr.
		 */
		const VertexAttributeBuffer* findVertexAttributeBuffer(const std::string& id) const;

		/**
		 * @param id name of the vertex attribute
		 * @return reference to the attribute buffer. If not found, the function will assert.
		 */
		VertexAttributeBuffer& getVertexAttributeBuffer(const std::string& id);

		/**
		 * Creates an index buffer is one does not exist, otherwise returns the existing buffer.
		 * @param index lookup value of the index buffer to get or create.
		 * @return an already existing or new index buffer.
		 */
		IndexBuffer& getOrCreateIndexBuffer(int index);

		/**
		 * @param index lookup value of the index buffer to get.
		 * @return The indexbuffer if one is created, if no index buffer exists, null is returned.
		 */
		const IndexBuffer& getIndexBuffer(int index) const;

	private:

		using AttributeMap = std::unordered_map<std::string, std::unique_ptr<VertexAttributeBuffer>>;
		AttributeMap								mAttributes;		///< Map from vertex attribute ID to buffer
		std::vector<std::unique_ptr<IndexBuffer>>	mIndexBuffers;		///< Index buffers
	};
} // opengl
