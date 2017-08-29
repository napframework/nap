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
		 * Adds a set of indices to the mesh. Without indices the regular
		 * array draw method applies, with indices the mesh will be drawn
		 * using the connectivity described by the indices
		 * @param count: The number of indices that will be copied
		 * @param indices: The array of indices that will describe mesh connectivity
		 */
		void setIndices(const std::vector<unsigned int>& indices);

		/**
		* @return The indexbuffer if set, otherwise nullptr.
		*/
		const IndexBuffer* getIndexBuffer() const;

	private:

		using AttributeMap = std::unordered_map<VertexAttributeID, std::unique_ptr<VertexAttributeBuffer>>;
		AttributeMap	mAttributes;
		IndexBuffer		mIndexBuffer;
	};
} // opengl
