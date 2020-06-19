#pragma once

#include "mesh.h"

namespace nap
{
	class Core;
	/**
	 * Represent a mesh that is loaded from a binary file. 
	 * A MeshInstance is loaded from binary file that is stored internally.
	 */
	class NAPAPI MeshFromFile : public IMesh
	{
		RTTI_ENABLE(IMesh)
	
	public:
		MeshFromFile(Core& core);

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

		std::string		mPath;								///< Property: 'Path' path to the file on disk
		EMeshDataUsage	mUsage = EMeshDataUsage::Static;	///< Property: 'Usage' specifies the way the mesh is used, allows the driver to optimize memory if necessary

	private:
		RenderService*						mRenderService;
		std::unique_ptr<MeshInstance>		mMeshInstance;
	};
}

