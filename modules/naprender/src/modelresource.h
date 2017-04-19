#pragma once

#include <nap/resource.h>
#include <nmodel.h>

namespace nap
{
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
		// Default constructor
		ModelResource() = default;

		virtual bool init(InitResult& initResult) override;

		/**
		 * @return the mesh display name
		 */
		virtual const std::string getDisplayName() const override;

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
		 * @return the mesh @index of the model, nullptr if out of range or invalid
		 * @param index the index of the mesh managed by this model
		 */
		opengl::Mesh* getMesh(unsigned int index) const;

		Attribute<std::string> mModelPath = { this, "mModelPath", "" };

	private:
		// Name of mesh
		std::string				mDisplayName;

		// opengl mesh object
		mutable opengl::Model	mModel;
	};


} // nap

RTTI_DECLARE(nap::ModelResource)
