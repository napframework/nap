#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <mesh.h>

namespace nap
{
	/**
	 * videomesh
	 */
	class VideoMeshFromFile : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		virtual ~VideoMeshFromFile();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* @return MeshInstance as created during init().
		*/
		virtual MeshInstance& getMeshInstance() override						{ return *mMeshInstance; }

		/**
		* @return MeshInstance as created during init().
		*/
		virtual const MeshInstance& getMeshInstance() const override			{ return *mMeshInstance; }


		std::string mPath;														///< Property: "Path" to mesh on disk

	private:
		std::unique_ptr<MeshInstance> mMeshInstance;							///< Holds the loaded mesh
		nap::VertexAttribute<glm::vec3>* mDirectionAttribute = nullptr;			///< Displacement direction attribute
		nap::VertexAttribute<glm::vec3>* mNormalAttribute = nullptr;			///< Normal attribute used for light calculations
		nap::VertexAttribute<glm::vec3>* mUVAttribute = nullptr;				///< UV attribute
		nap::VertexAttribute<glm::vec3>* mPositionAttribute = nullptr;			///< Vertex position attribute
		nap::VertexAttribute<glm::vec3>* mUVCenterAttribute = nullptr;			///< UV center Attribute
	};
}
