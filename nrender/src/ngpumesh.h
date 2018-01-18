#pragma once

// Local Includes
#include "nvertexarrayobject.h"
#include "nglutils.h"

// External Includes
#include <memory>

namespace opengl
{
	/**
	 * Defines the GPU data of a polygonal mesh
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
		* @param id: name of the vertex attribute
		* @param components: number of component per element (for instance, 3 for vector with 3 floats)
		* @param data: Pointer to array containing attribute data.
		*/
		void addVertexAttribute(const std::string& id, GLenum type, unsigned int numComponents, GLenum usage);

		/**
		* @return Returns reference to the attribute buffer if found, otherwise nullptr.
		* @param id: name of the vertex attribute
		*/
		const VertexAttributeBuffer* findVertexAttributeBuffer(const std::string& id) const;

		/**
		* @return Returns reference to the attribute buffer. If not found, the function will assert.
		* @param id: name of the vertex attribute
		*/
		VertexAttributeBuffer& getVertexAttributeBuffer(const std::string& id);

		/**
		 * Creates an index buffer is one does not exist, otherwise returns the existing buffer.
		* @return A valid index buffer.
		*/
		IndexBuffer& getOrCreateIndexBuffer();

		/**
		* @return The indexbuffer if one is created, if no index buffer exists, null is returned.
		*/
		const IndexBuffer* getIndexBuffer() const;

	private:

		using AttributeMap = std::unordered_map<std::string, std::unique_ptr<VertexAttributeBuffer>>;
		AttributeMap					mAttributes;		///< Map from vertex attribute ID to buffer
		std::unique_ptr<IndexBuffer>	mIndexBuffer;		///< Index buffer
	};
} // opengl
