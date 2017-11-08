#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <mesh.h>

namespace nap
{
	/**
	 * artnetmeshfromfile
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
