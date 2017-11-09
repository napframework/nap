#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <mesh.h>

namespace nap
{
	/**
	 * ArtnetMeshFromFile
	 *
	 * Loads an FBX mesh that contains the art net LED addressing as RGB color
	 * where R = channel, G = universe and B = subnet. 
	 * The range of the channel is mapped from 0-1 to 0 - 511
	 * The range of the universe is mapped from 0-1 to 0-15
	 * The range of the subnet is mapped from 0-1 to 0-15
	 *
	 * These color codes are converted and added as int attributes to the mesh as
	 * "channel", "universe" and "subnet". This frees the color slot to be used 
	 * as the actual color when drawing to screen. The mesh expects unique triangles
	 * where the vertices are not shared. This allows for every triangle to have it's
	 * own unique channel, subnet and universe. The mesh won't load if any discrepancies
	 * per triangle are detected, ie: the 3 attributes that make up the triangle
	 * are not the same
	 */
	class ArtnetMeshFromFile : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		/**
		* Loads model from file.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* @return MeshInstance as created during init().
		*/
		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }

		/**
		* @return MeshInstance as created during init().
		*/
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

		std::string		mPath;

	private:
		std::unique_ptr<MeshInstance>		mMeshInstance;

		nap::VertexAttribute<glm::vec4>*	mColorAttribute = nullptr;
		nap::VertexAttribute<int>*			mChannelAttribute = nullptr;
		nap::VertexAttribute<int>*			mUniverseAttribute = nullptr;
		nap::VertexAttribute<int>*			mSubnetAttribute = nullptr;
	};
}
