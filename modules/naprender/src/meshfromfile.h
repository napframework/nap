#pragma once

#include "mesh.h"

namespace nap
{
	class NAPAPI MeshFromFile : public IMesh
	{
		RTTI_ENABLE(IMesh)
	
	public:
		/**
 		 * Loads model from file.
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

		virtual MeshInstance& getMeshInstance() { return *mMeshInstance; }
		virtual const MeshInstance& getMeshInstance() const { return *mMeshInstance; }

		std::string		mPath;

	private:
		std::unique_ptr<MeshInstance>		mMeshInstance;
	};

}

