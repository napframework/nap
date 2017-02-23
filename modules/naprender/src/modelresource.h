#pragma once

#include <nap/resource.h>
#include <nmodel.h>

namespace nap
{
	class ModelResourceLoader;

	/**
	 * Wraps an opengl model object
	 * Note that the model is not initialized when this resource is created
	 * This is deferred to actual rendering because of gl initialization
	 * Every model resource holds one or more opengl mesh objects
	 */
	class ModelResource : public Resource
	{
		friend class MeshResourceLoader;
		RTTI_ENABLE_DERIVED_FROM(Resource)
	public:
		// Constructor
		ModelResource(const std::string& meshPath);
		
		// Default constructor
		ModelResource() = default;

		/**
		 * @return the mesh display name
		 */
		virtual const std::string& getDisplayName() const override						{ return mDisplayName; }

		/**
		 * @return the opengl mesh that can be drawn to screen or buffer
		 * Note that this will also initialize the mesh on the GPU
		 * if the mesh can't be allocated a warning will be raised
		 * in that case future binding calls won't work
		 */
		opengl::Model& getModel() const;

		/**
		 * @return the number of meshes in the model, 0 if model could not be loaded or is invalid
		 */
		unsigned int getMeshCount() const;

		/**
		 * @return if the model contains any mesh data
		 */
		bool isEmpty() const;

		/**
		 * Explicitly load the model, reloads if necessary
		 * Note that this call needs an active opengl context
		 */
		void load() const;

		/**
		 * @return the mesh @index of the model, nullptr if out of range or invalid
		 * @param index the index of the mesh managed by this model
		 */
		opengl::Mesh* getMesh(unsigned int index) const;

	private:
		// Path to mesh on disk
		std::string				mModelPath;

		// Name of mesh
		std::string				mDisplayName;

		// opengl mesh object
		mutable opengl::Model	mModel;

		// If the mesh has been loaded
		mutable bool			mLoaded = false;
	};


	/**
	 * Creates the model resource
	 * For a list of supported 3d model formats check: http://www.assimp.org/lib_html/index.html
	 */
	class ModelResourceLoader : public ResourceLoader
	{
		RTTI_ENABLE_DERIVED_FROM(ResourceLoader)
	public:
		ModelResourceLoader();

		/**
		 * @return all supported model extensions
		 */
		static const std::vector<std::string>& getSupportedModelExtensions();

		/**
		 *  Creates a model resource
		 * @return the newly created resource, nullptr if not successful
		 * @param resourcePath path to the model resource to load
		 */
		virtual std::unique_ptr<Resource> loadResource(const std::string& resourcePath) const override;

	};
} // nap

RTTI_DECLARE(nap::ModelResource)
RTTI_DECLARE(nap::ModelResourceLoader)
