#include "nmodel.h"

namespace opengl
{
	// returns the associated Mesh, nullptr if mesh at index does not exist
	Mesh* Model::getMesh(unsigned int index) const
	{
		if (index >= mMeshes.size())
		{
			printMessage(MessageType::ERROR, "mesh index out of range");
			return nullptr;
		}
		return mMeshes[index].get();
	}


	// Returns total number of managed meshes
	unsigned int Model::getMeshCount() const
	{
		return static_cast<unsigned int>(mMeshes.size());
	}


	// Returns total number of vertices
	unsigned int Model::getVertCount() const
	{
		unsigned int count(0);
		for (const auto& mesh : mMeshes)
			count += mesh->getVertCount();
		return count;
	}


	// Draws all meshes to currently active context
	void Model::draw()
	{
		for (auto& mesh : mMeshes)
		{
			mesh->draw();
		}
	}


	// Clears all associated mesh data
	void Model::clear()
	{
		mMeshes.clear();
	}


	// Adds a mesh, taking ownership
	void Model::addMesh(Mesh* mesh)
	{
		// Create unique ptr and add
		mMeshes.emplace_back(std::move(std::unique_ptr<Mesh>(mesh)));
	}


	// Return if the model has any mesh data
	bool Model::isEmpty() const
	{
		return mMeshes.size() == 0;
	}

}