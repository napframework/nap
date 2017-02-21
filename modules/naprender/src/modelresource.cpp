// Local Includes
#include "modelresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>
#include <nmodelutils.h>

namespace nap
{
	// Constructor
	ModelResource::ModelResource(const std::string& meshPath)
	{
		mModelPath = meshPath;
		mDisplayName = getFileNameWithoutExtension(meshPath);
		assert(mDisplayName != "");
	}


	// Returns associated mesh
	// This call will allocate the mesh on the gpu if not done already
	opengl::Model& ModelResource::getModel() const
	{
		if (!mLoaded)
		{
			load();
		}
		return mModel;
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


	// Loads the model resource
	void ModelResource::load() const
	{
		if (!opengl::loadModel(mModel, getResourcePath()))
		{
			nap::Logger::warn("unable to load model: %s", mModelPath.c_str());
		}
		mLoaded = true;
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


	//////////////////////////////////////////////////////////////////////////
	// Model Resource Loader
	//////////////////////////////////////////////////////////////////////////

	// Returns all supported model extensions in one vector
	const std::vector<std::string>& ModelResourceLoader::getSupportedModelExtensions()
	{
		static std::vector<std::string> extensions;
		if (extensions.empty())
		{
			extensions = std::vector<std::string>
			{
				"fbx",
				"dae",
				"blend",
				"3ds",
				"ase",
				"obj",
				"ply",
				"dxf",
				"x",
				"ac",
				"dxf",
				"off",
				"ter",
				"mdl",
				"hmp",
				"lwo",
				"lws",
				"lxo",
				"csm"
			};
		}
		return extensions;
	}


	// Creates the model resource
	std::unique_ptr<Resource> ModelResourceLoader::loadResource(const std::string& resourcePath) const
	{
		return std::make_unique<ModelResource>(resourcePath);
	}


	// Constructor registers available model extensions
	ModelResourceLoader::ModelResourceLoader()
	{
		const std::vector<std::string>& extensions = getSupportedModelExtensions();
		for (const auto& ext : extensions)
		{
			addFileExtension(ext);
		}
	}

}

RTTI_DEFINE(nap::ModelResource)
RTTI_DEFINE(nap::ModelResourceLoader)