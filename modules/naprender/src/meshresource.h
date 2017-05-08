#pragma once

#include <nap/resource.h>
#include <nmesh.h>
#include <memory>

namespace nap
{
	class MeshResource : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// Default constructor
		MeshResource() = default;

		/**
 		 * Loads model from file.
 		 */
		virtual bool init(InitResult& initResult) override;

		/**
 		 * Performs commit or rollback of changes made in init.
 		 */
		virtual void finish(Resource::EFinishMode mode) override;

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
		opengl::Mesh& getMesh() const;

		std::string				mPath;

	protected:
		// Name of mesh
		std::string				mDisplayName;

		// opengl mesh object
		std::unique_ptr<opengl::Mesh>	mMesh;
		std::unique_ptr<opengl::Mesh>	mPrevMesh;
	};

	class CustomMeshResource : public MeshResource
	{
	public:
		virtual bool init(InitResult& initResult) override;

	public:
		std::unique_ptr<opengl::Mesh> mCustomMesh = nullptr;
	};

} // nap

