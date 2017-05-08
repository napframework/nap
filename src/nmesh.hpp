#pragma once

namespace opengl
{
	// Creates or updates a vertex container associated with this mesh
	template <typename T>
	void Mesh::updateVertexContainer(std::unique_ptr<TypedVertexContainer<T>>& location, unsigned int components, unsigned int verts, const T* data)
	{
		// Check if exists, if not create, move and add
		if (location == nullptr)
		{
			location = std::make_unique<TypedVertexContainer<T>>(components, verts);
			mObject.addVertexBuffer(*(location->getVertexBuffer()));
		}
		else
		{
			printMessage(MessageType::WARNING, "mesh location already contains vertex buffer data");
			printMessage(MessageType::WARNING, "overriding existing data");
		}

		// Copy our data
		if (!location->copyData(components, verts, data))
		{
			printMessage(MessageType::ERROR,   "unable to copy data in to mesh container");
			return;
		}

		// Synchronize on success (data in CPU memory will be uploaded to GPU)
		location->sync();
	}
}
