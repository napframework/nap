// Local includes
#include "nvertexcontainer.h"
#include "nglutils.h"

namespace opengl
{
	// Clear vertex container (data in memory)
	// When the vertex buffer on the GPU is cleared (ie, destroyed) it will delete itself on the GPU
	VertexContainer::~VertexContainer()
	{
		// Clear data
		clear();
	}

	// Allocate memory based on provided settings
	bool VertexContainer::allocateMemory()
	{
		// Make 
		if (!mSettings.isValid())
		{
			printMessage(MessageType::ERROR, "can't allocate vertex container memory, invalid buffer settings");
			return false;
		}

		// Clear existing data if already associated
		if (hasData())
		{
			printMessage(MessageType::WARNING, "vertex container already contains data, clearing...");
			clear();
		}

		// Allocate
		mData = malloc(getSize());
		return true;
	}


	// Updates settings and allocated memory
	bool VertexContainer::allocateMemory(GLenum type, unsigned int components, unsigned int verts)
	{
		VertexBufferSettings new_settings(type, components, verts);
		if (!new_settings.isValid())
		{
			printMessage(MessageType::ERROR, "unable to allocate memory, invalid vertex buffer settings provided");
			return false;
		}

		// Store new settings
		mSettings = new_settings;

		// Allocate memory
		return allocateMemory();
	}


	// Copies source in to mData
	bool VertexContainer::copyData(void* source)
	{
		if (!mSettings.isValid())
		{
			printMessage(MessageType::ERROR, "can't copy vertex data, invalid settings");
			return false;
		}

		// Clear existing data
		clear();

		// Allocate new set of memory
		allocateMemory();

		// Copy
		memcpy(mData, source, getSize());
		return true;
	}


	// Applies new settings and performs copy
	bool VertexContainer::copyData(GLenum type, unsigned int components, unsigned int verts, void* source)
	{
		// Check settings
		VertexBufferSettings new_settings(type, components, verts);
		if (!new_settings.isValid())
		{
			printMessage(MessageType::ERROR, "unable to copy data in to vertex container, invalid buffer settings provided!");
			return false;
		}
		
		// Copy over settings
		mSettings = new_settings;

		// Copy data block
		return copyData(source);
	}


	// Sets data associated with this container
	// Note that not matching memory size requirements will cause
	// all sorts of unexpected side-effcts or even crashes
	// Make sure to match container settings before calling this function!
	// USE WITH CAUTION!
	void VertexContainer::setData(void* data)
	{
		clear();
		mData = data;
	}


	// Clears all data associated with the container
	void VertexContainer::clear()
	{
		if (mData == nullptr)
			return;

		// Free memory
		free(mData);

		// Clear data
		mData = nullptr;
	}


	// Returns the size in bytes of the data associated with the container
	size_t VertexContainer::getSize() const
	{
		// Return size
		return mSettings.getSize();
	}


	// Returns length of array without taking in to account data size
	size_t VertexContainer::getLength() const
	{
		return mSettings.getLength();
	}


	// Creates and returns the vertex buffer associated with this container
	// nullptr if the buffer can't be created
	VertexBuffer* VertexContainer::getVertexBuffer()
	{
		// Create one if we don't have one yet
		if (mGPUBuffer == nullptr)
		{
			mGPUBuffer = std::move(createVertexBuffer());
		}
		return mGPUBuffer.get();
	}


	//Uploads the data on to the GPU
	void VertexContainer::update()
	{
		// Make sure we have data before updating
		if (!hasData())
		{
			printMessage(MessageType::ERROR, "unable to update vertex buffer, no data associated with vertex container");
			return;
		}

		// Retrieve vertex buffer
		opengl::VertexBuffer* buffer = getVertexBuffer();
		if (buffer == nullptr)
		{
			printMessage(MessageType::ERROR, "unable to update vertex buffer, can't retrieve container associated GPU buffer");
			return;
		}

		// Set data (unsafe, make sure buffer and container settings match)
		buffer->setData(mData);
	}


	// Synchronize GPU vertex buffer with data stored in memory
	void VertexContainer::sync()
	{
		// Make sure we have data before updating
		if (!hasData())
		{
			printMessage(MessageType::ERROR, "unable to sync vertex buffer, no data associated with vertex container");
			return;
		}

		opengl::VertexBuffer* buffer = getVertexBuffer();
		if (buffer == nullptr)
		{
			printMessage(MessageType::ERROR, "unable to sync vertex buffer, can't retrieve container associated GPU buffer");
			return;
		}
		buffer->setData(mSettings, mData);
	}


	// Helper function to create a new GPU bound vertex buffer
	std::unique_ptr<VertexBuffer> VertexContainer::createVertexBuffer()
	{
		if (!mSettings.isValid())
		{
			printMessage(MessageType::ERROR, "unable to create vertex buffer, invalid vertex container settings");
			return nullptr;
		}
		
		// Create and initialize
		std::unique_ptr<VertexBuffer> new_buffer = std::make_unique<VertexBuffer>(mSettings);
		new_buffer->init();

		// Set data if data is available
		if(hasData())
			new_buffer->setData(mData);
		return new_buffer;
	}

	//////////////////////////////////////////////////////////////////////////
	// Template Specialization
	//////////////////////////////////////////////////////////////////////////
    template<>
	GLenum TypedVertexContainer<float>::getType() const		{ return GL_FLOAT; }

    template<>
    GLenum TypedVertexContainer<int>::getType()   const		{ return GL_INT; }

    template<>
    GLenum TypedVertexContainer<int8_t>::getType() const	{ return GL_BYTE; }

    template<>
    GLenum TypedVertexContainer<double>::getType() const	{ return GL_DOUBLE; }


} // opengl