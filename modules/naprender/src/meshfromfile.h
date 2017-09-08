#pragma once

#include "mesh.h"

namespace nap
{
	/**
	 * Represent a mesh that is loaded from a binary file. 
	 * A MeshInstance is loaded from binary file that is stored internally.
	 */
	class NAPAPI MeshFromFile : public IMesh
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
	};

}

