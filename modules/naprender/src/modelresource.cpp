// Local Includes
#include "modelresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>
#include <nmodelutils.h>

RTTI_BEGIN_CLASS(nap::ModelResource)
	RTTI_PROPERTY_FILE_LINK("mModelPath", &nap::ModelResource::mModelPath)
RTTI_END_CLASS

namespace nap
{
	// Returns associated mesh
	opengl::Model& ModelResource::getModel() const
	{
		return mModel;
	}

	bool ModelResource::init(InitResult& initResult)
	{
		if (!initResult.check(opengl::loadModel(mModel, mModelPath), "Unable to load model %s", mModelPath.c_str()))
			return false;

		return true;
	}

	const std::string ModelResource::getDisplayName() const
	{
		return getFileNameWithoutExtension(mModelPath);
	}

	// Returns number of meshes in the model
	unsigned int ModelResource::getMeshCount() const
	{
		return getModel().getMeshCount();
	}


	// If the model contains any mesh data
	bool ModelResource::isEmpty() const
	{
		return getModel().isEmpty();
	}

	// Returns the mesh @index
	opengl::Mesh* ModelResource::getMesh(unsigned int index) const
	{
		opengl::Mesh* mesh = getModel().getMesh(index);
		if (mesh == nullptr)
		{
			nap::Logger::warn("unable to fetch mesh at index: %d from model: %s", index, mModelPath.c_str());
		}
		return mesh;
	}

}

RTTI_DEFINE(nap::ModelResource)