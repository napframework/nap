// Local Includes
#include "nindexbuffer.h"
#include "ngltypes.h"

namespace opengl
{
	// Total size in bytes of index buffer
	std::size_t IndexBufferSettings::getSize() const
	{
		return getGLTypeSize(GL_UNSIGNED_INT) * mCount;
	}


	// If the index settings are valid
	bool IndexBufferSettings::isValid() const
	{
		return mCount != 0;
	}


	// Constructor
	IndexBuffer::IndexBuffer(const IndexBufferSettings& settings) : mSettings(settings)
	{}


	// Upload data
	void IndexBuffer::setData(unsigned int* indices)
	{
		if (!bind())
			return;

		if (!mSettings.isValid())
		{
			printMessage(MessageType::ERROR, "can't upload vertex index data, invalid index buffer settings");
			return;
		}
		else
		{
			glBufferData(getBufferType(), mSettings.getSize(), indices, mSettings.mUsage);
		}
		unbind();
	}


	// Update settings and upload data
	void IndexBuffer::setData(unsigned int* indices, std::size_t count)
	{
		mSettings.mCount = count;
		setData(indices);
	}
}