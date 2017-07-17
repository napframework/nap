#pragma once

#include <nap/resource.h>
#include <nmesh.h>
#include <memory>
#include <nap/dllexport.h>

namespace nap
{
	class NAPAPI MeshResource : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		// Default constructor
		MeshResource() = default;

		/**
 		 * Load the mesh
 		 */
		virtual bool init(utility::ErrorState& errorState) = 0;

		/**
		 * @return the opengl mesh that can be drawn to screen or buffer
		 */
		opengl::Mesh& getMesh() const;

	protected:
		// opengl mesh object
		std::unique_ptr<opengl::Mesh>	mMesh;
	};


	class NAPAPI MeshFromFileResource : public MeshResource
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

