#pragma once

// Local Includes
#include "nvertexarrayobject.h"
#include "nglutils.h"

// External Includes
#include <memory>
#include <vector>

namespace opengl
{
	/**
	 * Defines a polygonal mesh
	 * Every mesh has a number of vertex attributes that are identified through an ID.
	 */
	class GPUMesh
	{
	public:
		using VertexAttributeID = std::string;

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
		void addVertexAttribute(const VertexAttributeID& id, GLenum type, unsigned int numComponents, unsigned int numVertices, GLenum usage);

		/**
		* @return Returns reference to the attribute buffer if found, otherwise nullptr.
		* @param id: name of the vertex attribute
		*/
		const VertexAttributeBuffer* findVertexAttributeBuffer(const VertexAttributeID& id) const;

		/**
		* @return Returns reference to the attribute buffer. If not found, the function will assert.
		* @param id: name of the vertex attribute
		*/
		const VertexAttributeBuffer& getVertexAttributeBuffer(const VertexAttributeID& id) const;

		/**
		* @return The indexbuffer if one is created, otherwise null.
		*/
		IndexBuffer& getOrCreateIndexBuffer();

		/**
		* @return The indexbuffer if one is created, otherwise null.
		*/
		const IndexBuffer* getIndexBuffer() const;

	private:

		using AttributeMap = std::unordered_map<VertexAttributeID, std::unique_ptr<VertexAttributeBuffer>>;
		AttributeMap					mAttributes;
		std::unique_ptr<IndexBuffer>	mIndexBuffer;
	};
} // opengl
