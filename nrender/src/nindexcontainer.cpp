#include "nindexcontainer.h"

namespace opengl
{
#if 0
	// Set new index data
	void IndexContainer::copyData(const VertexIndices& indices)
	{
		mIndices = indices;
	}

	// Copy all contents in to vector
	void IndexContainer::copyData(unsigned int count, const unsigned int* indices)
	{
		clear();
		mIndices.insert(mIndices.end(), indices, &indices[count]);
	}

	// Add a single index
	void IndexContainer::addIndex(unsigned int index)
	{
		mIndices.emplace_back(index);
	}

	// Clear indices
	void IndexContainer::clear()
	{
		mIndices.clear();
	}


	// Return / Create the index buffer
	IndexBuffer* IndexContainer::getIndexBuffer()
	{
		// Create one if we don't have one yet
		if (mIndexBuffer == nullptr)
		{
			mIndexBuffer = createIndexBuffer();
		}
		return mIndexBuffer.get();
	}


	// Update without checking settings
	void IndexContainer::update()
	{
		// Make sure we have data before updating
		if (!hasData())
		{
			printMessage(MessageType::ERROR, "unable to update vertex buffer, no data associated with vertex container");
			return;
		}

		// Retrieve vertex buffer
		opengl::IndexBuffer* buffer = getIndexBuffer();
		if (buffer == nullptr)
		{
			printMessage(MessageType::ERROR, "unable to update index buffer, can't retrieve container associated GPU buffer");
			return;
		}

		// Set data (unsafe, make sure buffer and container settings match)
		buffer->setData(mIndices);
	}


	// Sync using new index settings
	void IndexContainer::sync()
	{
		// Make sure we have data before updating
		if (!hasData())
		{
			printMessage(MessageType::ERROR, "unable to update vertex buffer, no data associated with vertex container");
			return;
		}

		// Retrieve vertex buffer
		opengl::IndexBuffer* buffer = getIndexBuffer();
		if (buffer == nullptr)
		{
			printMessage(MessageType::ERROR, "unable to update index buffer, can't retrieve container associated GPU buffer");
			return;
		}

		// Set data (unsafe, make sure buffer and container settings match)
		buffer->setData(&mIndices.front(), getCount());
	}


	// Create the index buffer
	std::unique_ptr<IndexBuffer> IndexContainer::createIndexBuffer()
	{
		// Create and initialize
		std::unique_ptr<IndexBuffer> new_buffer = std::make_unique<IndexBuffer>();
		new_buffer->init();

		// Set data if data is available
		if (hasData())
		{
			new_buffer->setData(&mIndices.front(), getCount());
		}
		return new_buffer;
	}
#endif
}
