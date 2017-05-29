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
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the mesh display name
		 */
		virtual const std::string getDisplayName() const override;

		/**
		 * @return the opengl mesh that can be drawn to screen or buffer
		 */
		opengl::Mesh& getMesh() const;

		std::string				mPath;

	protected:
		// Name of mesh
		std::string				mDisplayName;

		// opengl mesh object
		std::unique_ptr<opengl::Mesh>	mMesh;
	};


	class CustomMeshResource : public MeshResource
	{
	public:
		virtual bool init(utility::ErrorState& errorState) override;

	public:
		std::unique_ptr<opengl::Mesh> mCustomMesh = nullptr;
	};

} // nap

