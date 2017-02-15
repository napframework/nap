#include "nvertexarrayobject.h"
#include "nglutils.h"

namespace opengl
{
	// Destructor
	VertexArrayObject::~VertexArrayObject()
	{
		if (isAllocated())
			glDeleteVertexArrays(1, &mId);
	}


	// Creates the vertex buffer
	void VertexArrayObject::init()
	{
		if (isAllocated())
		{
			printMessage(MessageType::WARNING, "vertex array object already allocated, generating new one");
			printMessage(MessageType::WARNING, "the previous vertex array object id will be invalidated");
			glDeleteVertexArrays(1, &mId);
		}
		glGenVertexArrays(1, &mId);
	}


	// Binds the vertex buffer
	bool VertexArrayObject::bind()
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to bind vertex array object: object is not allocated");
			return false;
		}
		glBindVertexArray(mId);
		return true;
	}


	// Unbinds the vertex buffer
	bool VertexArrayObject::unbind()
	{
		if (!isAllocated())
		{
			printMessage(MessageType::ERROR, "unable to unbind vertex array object: object is not allocated");
			return false;
		}
		glBindVertexArray(0);
		return true;
	}


	// Checks if the buffer is allocated on the GPU
	bool VertexArrayObject::isAllocated() const
	{
		return mId != 0;
	}


	//  Draws all the vertex data associated with this buffer object
	void VertexArrayObject::draw(int count /*= -1*/)
	{
		if (!bind())
		{
			printMessage(MessageType::ERROR, "can't draw vertex array object");
			return;
		}

		// Get gl draw mode
		GLenum draw_mode = getGLMode(mDrawMode);

		// Draw with or without using indices
		if (mIndexBuffer != nullptr)
		{
			if (!mIndexBuffer->bind())
			{
				printMessage(MessageType::ERROR, "can't draw vertex array object, unable to bind index buffer");
				return;
			}

			// Get number of indices to draw
			GLsizei draw_count = count < 0 ? static_cast<GLsizei>(mIndexBuffer->getCount()) : static_cast<GLsizei>(count);
			
			// Draw vertices using connectivity list
			glDrawElements(draw_mode, draw_count, mIndexBuffer->getType(), 0);
		}
		else
		{
			// Get number of verts to draw
			GLsizei draw_count = count < 0 ? static_cast<GLsizei>(mVertCount) : static_cast<GLsizei>(count);

			// Draw all arrays managed by this vertex array object
			glDrawArrays(draw_mode, 0, draw_count);
		}

		// Unbind object
		unbind();
	}

	// Specify which index buffer to use with this vertex array object
	bool VertexArrayObject::setIndexBuffer(IndexBuffer& buffer)
	{
		if (!buffer.isAllocated())
		{
			printMessage(MessageType::ERROR, "can't add index buffer to vertex array object, index buffer is not allocated");
			return false;
		}
		mIndexBuffer = &buffer;
		return true;
	}

	//Adds and binds a vertex buffer to this vertex array object
	bool VertexArrayObject::addVertexBuffer(unsigned int index, VertexBuffer& buffer)
	{
		auto it = mBindings.find(index);
		if (it != mBindings.end())
		{
			printMessage(MessageType::ERROR, "vertex array object already has a vertex buffer bound with index: %d", index);
			printMessage(MessageType::WARNING, "skipping vertex buffer assignment for index: %d", index);
			return false;
		}

		// Make sure we can bind our object
		if (!bind())
		{
			printMessage(MessageType::ERROR, "can't add vertex buffer, unable to bind vertex array object");
			return false;
		}
		
		// Make sure buffer to add is allocated
		if (!buffer.isAllocated())
		{
			printMessage(MessageType::ERROR, "can't add vertex buffer, buffer is not allocated");
			return false;
		}

		if (buffer.getComponentCount() == 0)
		{
			printMessage(MessageType::ERROR, "can't add vertex buffer, invalid number of components (0)");
			return false;
		}

		// Update internal vertex count
		unsigned int new_vert_count = buffer.getVertCount();
		
		// If the buffer to add has a different vert count, compensate internal count!
		if (mBindings.size() > 0 && new_vert_count != mVertCount)
		{
			printMessage(MessageType::WARNING, "buffer vertex count: %d differs from previously defined vertex count: %d", buffer.getVertCount(), mVertCount);
			printMessage(MessageType::WARNING, "picking lowest denominator");
			new_vert_count = new_vert_count < mVertCount ? new_vert_count : mVertCount;
		}

		// Update vert count
		mVertCount = new_vert_count;

		// Bind buffer
		buffer.bind();

		// Enable vertex attribute array relative to this vertex array object
		glEnableVertexAttribArray(index);

		// Set data pointer so GL knows how to assign to shader later on
		glVertexAttribPointer(index, buffer.getComponentCount(), buffer.getType(), GL_FALSE, 0, 0);

		// Unbind buffer
		buffer.unbind();

		// Unbind this object
		unbind();

		// Add index
		mBindings.emplace(std::make_pair(index, &buffer));

		return true;
	}


	// Adds a vertex buffer at next available id
	unsigned int VertexArrayObject::addVertexBuffer(VertexBuffer& buffer)
	{
		// Try to find a free id
		unsigned int found_id(0);
		while (true)
		{
			if (mBindings.find(found_id) == mBindings.end())
				break;
			found_id++;
		}

		// Add vertex buffer and return new index on success
		if (addVertexBuffer(found_id, buffer))
			return found_id;
		
		// Otherwise -1 for failure
		return -1;
	}


	// return the vertex buffer binding index associated with this VAO
	int VertexArrayObject::getVertexBufferIndex(const VertexBuffer& buffer) const
	{
		for (const auto& binding : mBindings)
		{
			if (binding.second == &buffer)
				return binding.first;
		}
		printMessage(MessageType::WARNING, "unable to find vertex buffer index that is bound with vertex array object");
		return -1;
	}


	// Sets the draw mode
	void VertexArrayObject::setDrawMode(DrawMode mode)
	{
		if (mode == DrawMode::UNKNOWN)
		{
			opengl::printMessage(MessageType::ERROR, "unable to set draw mode, mode unknown");
			return;
		}
		mDrawMode = mode;
	}
}