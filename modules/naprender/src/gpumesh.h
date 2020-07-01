#pragma once

// Local Includes
#include "vertexbuffer.h"
#include "indexbuffer.h"

// External Includes
#include <memory>
#include <string>
#include "vulkan/vulkan_core.h"
#include <unordered_map>
#include "utility/dllexport.h"

namespace nap
{
	/**
	 * Defines the GPU data of a polygonal mesh.
	 */
	class NAPAPI GPUMesh
	{
	public:
		GPUMesh(RenderService& renderService, EMeshDataUsage inUsage);

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
		void addVertexBuffer(const std::string& id, VkFormat format);

		/**
		 * @param id name of the vertex attribute
		 * @return reference to the attribute buffer if found, otherwise nullptr.
		 */
		const VertexBuffer* findVertexBuffer(const std::string& id) const;

		/**
		 * @param id name of the vertex attribute
		 * @return reference to the attribute buffer. If not found, the function will assert.
		 */
		VertexBuffer& getVertexBuffer(const std::string& id);

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
		using AttributeMap = std::unordered_map<std::string, std::unique_ptr<VertexBuffer>>;
		RenderService*								mRenderService;
		AttributeMap								mAttributes;		///< Map from vertex attribute ID to buffer
		std::vector<std::unique_ptr<IndexBuffer>>	mIndexBuffers;		///< Index buffers
		EMeshDataUsage								mUsage = EMeshDataUsage::Static;
	};
}
