#pragma once

// Local Includes
#include "nindexbuffer.h"

// External Includes
#include <vector>

/**
 * IndexContainer
 *
 * Manages a set of indices that can be associated with a mesh
 * This object owns that index data. Index data describes the vertex
 * layout in order to draw a mesh and can greatly reduce the GPU memory 
 * footprint
 */
namespace opengl
{
	// Name for list of indices
	using VertexIndices = std::vector<unsigned int>;

	class IndexContainer
	{
	public:
		// Default Constructor
		IndexContainer() = default;

		// Don't allow copy, TODO: implement copy
		IndexContainer(const IndexContainer& other) = delete;
		IndexContainer& operator=(const IndexContainer& other) = delete;

		/**
		 * @return all the currently managed indices (CPU)
		 */
		const VertexIndices& getIndices() const						{ return mIndices; }

		/**
		 * Copies data from indices in to this index container
		 * Note that after this call the vertex buffer is automatically updated
		 * @param indices: the new indices to use
		 */
		void copyData(const VertexIndices& indices);

		/**
		* Copies data from indices in to this index container
		* Note that data set previously will be cleared
		* If no data is associated with the container new memory is allocated
		* Note that this call will update CPU memory, if  a GPU buffer has been created those settings will be invalidated
		* To make sure the GPU buffer is in sync with local memory -> call sync()
		*/
		void copyData(unsigned int count, const unsigned int* indices);

		/**
		 * Adds a new index to the current list of indices
		 * @param index: the index to add
		 */
		void addIndex(unsigned int index);

		/**
		 * Clears all indices
		 * Note that this does not update the GPU buffer
		 */
		void clear();

		/**
		 * If the container holds currently any index data
		 */
		bool hasData() const									{ return !mIndices.empty(); }

		/**
		* @return a GPU index buffer that holds the data associated with this container, nullptr if buffer can't be created
		* This function will create a buffer if no buffer has been created yet
		* If there is data associated with this container that data will be uploaded to the GPU when the buffer is created
		* If no data is set but the buffer is created call update to upload new data
		* When the data in the container changes (settings), make sure to change the settings for the buffer
		* When the CPU data has changed, call update to upload the changes on to the GPU
		* The returned buffer will be owned by this object and is deleted on object destruction
		*/
		IndexBuffer* getIndexBuffer();

		/**
		* @return the number of indices specified by this container
		* note that this number is related to the number of indices
		* on the CPU, the CPU and GPU buffers might be out of sync
		*/
		std::size_t getCount() const							{ return mIndices.size(); }

		/**
		* Uploads the data on to the GPU without synchronizing settings
		* The update will be based on the settings associated with this container
		* Data that does not match buffer settings will result in undefined behavior, possibly a crash;
		* Note that if no data is set or the container settings are invalid, this call will fail
		* If there's no index buffer one will be created by this call
		*/
		void update();

		/**
		* Updates the GPU settings based on current container settings and pushes data on to the GPU
		* Useful when a new set of data is copied with different settings and the GPU component is out of sync
		* This call will apply settings and push a GPU update
		* Note that if no data is set or the container settings are invalid, this call will fail
		*/
		void sync();

	protected:
		/**
		 * Associated GPU buffer
		 */
		std::unique_ptr<IndexBuffer> mIndexBuffer = nullptr;
		
		// Helper function top create a new GPU bound index buffer
		std::unique_ptr<IndexBuffer> createIndexBuffer();

		// All currently managed indices
		VertexIndices mIndices;
	};
}
