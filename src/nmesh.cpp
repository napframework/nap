#include "nmesh.h"

namespace opengl
{
	// Creates the containing vertex array object that will hold all the vertex buffers
	void Mesh::init()
	{
		mObject.init();
	}


	// Adds vertex data
	void Mesh::copyVertexData(unsigned int vertices, float* data)
	{
		updateVertexContainer<float>(mVertices, 3, vertices, data);
	}


	// Adds normal data
	void Mesh::copyNormalData(unsigned int vertices, float* data)
	{
		updateVertexContainer<float>(mNormals, 3, vertices, data);
	}


	// Adds color data
	void Mesh::copyColorData(unsigned int components, unsigned int vertices, float* data)
	{
		std::unique_ptr<FloatVertexContainer> container = nullptr;
		updateVertexContainer(container, components, vertices, data);
		mColors.emplace_back(std::move(container));
	}


	// Adds uv data
	void Mesh::copyUVData(unsigned int components, unsigned int vertices, float* data)
	{
		std::unique_ptr<FloatVertexContainer> container = nullptr;
		updateVertexContainer(container, components, vertices, data);
		mUvs.emplace_back(std::move(container));
	}


	// Draw mesh object
	void Mesh::draw(GLenum mode /*= GL_TRIANGLES*/)
	{
		mObject.draw(mode);
	}

	
	// Utility that is used for retrieving the binding for @container
	int Mesh::getContainerBindingIndex(VertexContainer* container) const
	{
		if (container == nullptr)
		{
			printMessage(MessageType::WARNING, "unable to retrieve vertex buffer binding index, vertex container has no data");
			return -1;
		}
		return mObject.getVertexBufferIndex(*(container->getVertexBuffer()));
	}


	// Return vertex buffer index
	int Mesh::getVertexBufferIndex() const
	{
		return getContainerBindingIndex(mVertices.get());
	}


	// Return normal buffer index
	int Mesh::getNormalBufferIndex() const
	{
		return getContainerBindingIndex(mNormals.get());
	}


	// Return the color buffer binding index, -1 if not found
	int Mesh::getColorBufferIndex(unsigned int colorChannel) const
	{
		if (colorChannel >= mColors.size())
		{
			printMessage(MessageType::WARNING, "unable to retrieve vertex buffer index, index out of bounds");
			return -1;
		}
		return getContainerBindingIndex(mColors[colorChannel].get());
	}


	// the uv buffer binding index, -1 if not found
	int Mesh::getUvBufferIndex(unsigned int uvChannel) const
	{
		if (uvChannel >= mUvs.size())
		{
			printMessage(MessageType::WARNING, "unable to retrieve vertex buffer index, index out of bounds");
			return -1;
		}
		return getContainerBindingIndex(mUvs[uvChannel].get());
	}

}