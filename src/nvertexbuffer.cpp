// Local Includes
#include "nvertexbuffer.h"
#include "nglutils.h"

// External Includes
#include <GL/glew.h>
#include <assert.h>

namespace opengl
{
	// return the number of verts multiplied by the number of components
	std::size_t VertexBufferSettings::getLength() const
	{
		return mComponents * mVerts;
	}

	// return the total size in bytes of the settings defined for the buffer
	std::size_t VertexBufferSettings::getSize() const
	{
		return getGLTypeSize(mType) * mComponents * mVerts;
	}

	// return if the settings are valid for the buffer to use
	bool VertexBufferSettings::isValid() const
	{
		return getGLTypeSize(mType) != 0 && mComponents != 0 && mVerts != 0;
	}


	//////////////////////////////////////////////////////////////////////////


	// Apply settings and store data pointer
	void VertexBuffer::setData(const VertexBufferSettings& settings, void* data)
	{
		mSettings = settings;
		setData(data);
	}


	// Apply settings and store data pointer
	void VertexBuffer::setData(GLenum type, unsigned int components, unsigned int verts, GLenum usage, void* data)
	{
		mSettings.mType = type;
		mSettings.mComponents = components;
		mSettings.mVerts = verts;
		mSettings.mUsage = usage;
		setData(data);
	}


	// Uploads the data block to the GPU
	void VertexBuffer::setData(void* data)
	{
		if (!bind())
			return;

		if (!mSettings.isValid())
		{
			printMessage(MessageType::ERROR, "can't upload vertex buffer data, invalid vertex buffer settings");
		}
		else
		{
			glBufferData(getBufferType(), mSettings.getSize(), data, mSettings.mUsage);
			glAssert();
		}

		// Unbind buffer after setting data
		unbind();
	}

	//////////////////////////////////////////////////////////////////////////
	// Template Specialization
	//////////////////////////////////////////////////////////////////////////
    template<>
    GLenum TypedVertexBuffer<float>::getType() const		{ return GL_FLOAT; }

    template<>
    GLenum TypedVertexBuffer<int>::getType()   const		{ return GL_INT; }

    template<>
    GLenum TypedVertexBuffer<int8_t>::getType() const		{ return GL_BYTE; }

    template<>
    GLenum TypedVertexBuffer<double>::getType() const		{ return GL_DOUBLE; }

} // opengl