/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "vertexbuffer.h"
#include "indexbuffer.h"

// External Includes
#include <memory>
#include <string>
#include <vulkan/vulkan_core.h>
#include <unordered_map>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Defines the GPU data of a polygonal mesh.
	 * Every GPU mesh contains at least one, but often multiple, nap::VertexBuffer(s).
	 * A vertex buffer contains vertex data, for example: color, position, normal etc.
	 *
	 * At least one nap::IndexBuffer is required, but multiple are supported. 
	 * The index buffer controls how triangles are formed (from the vertex data) when rendered.
	 * A GPU mesh has more than 1 index buffer when it contains multiple shapes, one for each.
	 *
	 * Together they outline the functionality and capabilities of a mesh.
	 *
	 * A static mesh is updated (uploaded to) only once, a dynamic mesh can be updated more frequently but
	 * requires more resources and is 'generally' slower to draw, depending of the memory layout of the underlying hardware.
	 * It is not allowed to update a static mesh after the initial upload!
	 *
	 * Note that static meshes are often placed in a different cache on the GPU, not accessible by the CPU, which
	 * allows for faster drawing times. 'DynamicWrite' meshes are uploaded into shared CPU / GPU memory
	 * and are therefore slower to draw.
	 */
	class NAPAPI GPUMesh
	{
	public:
		/**
		 * @param renderService render backend.
		 * @param usage how the mesh data is used at runtime.
		 */
		GPUMesh(RenderService& renderService, EMeshDataUsage usage);

		// Default destructor
		virtual ~GPUMesh() = default;

		// Copy construction and copy assignment not allowed
		GPUMesh(const GPUMesh& other) = delete;
		GPUMesh& operator=(const GPUMesh& other) = delete;

		/**
		 * Creates and adds a new vertex buffer to the mesh.
		 * The new buffer has yet to be initialized before it can be updated. This can be checked with isInitialized().
		 * @param id name of the vertex buffer to create and add.
		 */
		template<typename ELEMENTTYPE>
		VertexBuffer& addVertexBuffer(const std::string& id);

		/**
		 * Finds a vertex buffer with the given name.
		 * @param id name of the vertex buffer
		 * @return reference to the vertex buffer if found, nullptr otherwise.
		 */
		const VertexBuffer* findVertexBuffer(const std::string& id) const;

		/**
		 * Returns a vertex buffer with the given name, asserts if not present.
		 * @param id name of the vertex buffer to get.
		 */
		VertexBuffer& getVertexBuffer(const std::string& id);

		/**
		 * Creates an index buffer if one does not exist, returns the existing buffer otherwise.
		 * If the buffer was created, it has yet to be initialized before it can be updated. This can be checked with isInitialized().
		 * @param index value of the index buffer to get or create.
		 * @return an already existing or new index buffer.
		 */
		IndexBuffer& getOrCreateIndexBuffer(int index);

		/**
		 * Returns an index buffer at the specified index, asserts if not present.
		 * @param index value of the index buffer to get.
		 * @return an index buffer.
		 */
		const IndexBuffer& getIndexBuffer(int index) const;

	private:
		using AttributeMap = std::unordered_map<std::string, std::unique_ptr<VertexBuffer>>;
		RenderService*								mRenderService;						///< Link to the render engine
		AttributeMap								mAttributes;						///< Map from vertex attribute ID to buffer
		std::vector<std::unique_ptr<IndexBuffer>>	mIndexBuffers;						///< Index buffers
		EMeshDataUsage								mUsage = EMeshDataUsage::Static;	///< By default a gpu mesh is static.
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename ELEMENTTYPE>
	VertexBuffer& GPUMesh::addVertexBuffer(const std::string& id)
	{
		std::unique_ptr<TypedVertexBuffer<ELEMENTTYPE>> vertex_buffer = std::make_unique<TypedVertexBuffer<ELEMENTTYPE>>(mRenderService->getCore(), mUsage);
		auto it = mAttributes.emplace(std::make_pair(id, std::move(vertex_buffer))).first;
		return *it->second;
	}
}
