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
 		 * Load the mesh
 		 */
		virtual bool init(utility::ErrorState& errorState) = 0;

		/**
		 * @return the mesh display name
		 */
		virtual const std::string getDisplayName() const override { return "mesh"; }

		/**
		 * @return the opengl mesh that can be drawn to screen or buffer
		 */
		opengl::Mesh& getMesh() const;

	protected:
		// opengl mesh object
		std::unique_ptr<opengl::Mesh>	mMesh;
	};

	class MeshFromFileResource : public MeshResource
	{
		RTTI_ENABLE(MeshResource)
	
	public:
		/**
 		 * Loads model from file.
 		 */
		virtual bool init(utility::ErrorState& errorState) override;
		std::string				mPath;
	};
} // nap

