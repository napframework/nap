#include "nmesh.h"
#include <assert.h>

namespace opengl
{
	// Constructor initializes object draw mode
	Mesh::Mesh()
	{
		mObject.setDrawMode(DrawMode::TRIANGLES);
	}


	// Adds vertex data
	void Mesh::copyVertexData(unsigned int vertices, const float* data)
	{
		updateVertexContainer<float>(mVertices, 3, vertices, data);
	}


	// Adds normal data
	void Mesh::copyNormalData(unsigned int vertices, const float* data)
	{
		updateVertexContainer<float>(mNormals, 3, vertices, data);
	}


	// Adds color data
	void Mesh::copyColorData(unsigned int components, unsigned int vertices, const float* data)
	{
		std::unique_ptr<FloatVertexContainer> container = nullptr;
		updateVertexContainer(container, components, vertices, data);
		mColors.emplace_back(std::move(container));
	}


	// Adds uv data
	void Mesh::copyUVData(unsigned int components, unsigned int vertices, const float* data)
	{
		std::unique_ptr<FloatVertexContainer> container = nullptr;
		updateVertexContainer(container, components, vertices, data);
		mUvs.emplace_back(std::move(container));
	}


	void Mesh::copyIndexData(unsigned int count, const unsigned int* data)
	{
		assert(count > 0);
		updateIndexContainer(mIndices, count, data);
	}


	// Draw mesh object
	void Mesh::draw()
	{
		mObject.draw();
	}


	// Update mesh indices
	void Mesh::updateIndexContainer(std::unique_ptr<IndexContainer>& location, unsigned int count, const unsigned int* data)
	{
		// Check if exists, if not create, move and add
		if (location == nullptr)
		{
			location = std::make_unique<IndexContainer>();
			mObject.setIndexBuffer(*(location->getIndexBuffer()));
		}

		// Copy our data
		location->copyData(count, data);

		// Synchronize on success (data in CPU memory will be uploaded to GPU)
		location->sync();
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