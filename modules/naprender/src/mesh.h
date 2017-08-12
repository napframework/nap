#pragma once

#include <nmesh.h>
#include <memory>
#include <utility/dllexport.h>
#include <rtti/rttiobject.h>

namespace nap
{
	class NAPAPI Mesh : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		// Default constructor
		Mesh() = default;

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


	class NAPAPI MeshFromFile : public Mesh
	{
		RTTI_ENABLE(Mesh)
	
	public:
		/**
 		 * Loads model from file.
 		 */
		virtual bool init(utility::ErrorState& errorState) override;
		std::string				mPath;
	};
} // nap

