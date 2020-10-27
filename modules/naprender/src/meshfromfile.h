/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "mesh.h"

namespace nap
{
	class Core;
	/**
	 * Loads a '.mesh' file from disk. After a successful load the mesh can be rendered.
	 * NAP uses its own binary mesh representation format: '.mesh', 'fbx' files are converted into '.mesh' files using the 'fbxconverter' tool.
	 * The 'fbxconverter' runs automatically after compilation and only converts '.fbx' files when new. 
	 * Alternatively you can run the tool from the command line. Type --help for instructions.
	 * If an '.fbx' file contains multiple meshes each mesh is stored into an individual '.mesh' file.
	 */
	class NAPAPI MeshFromFile : public IMesh
	{
		RTTI_ENABLE(IMesh)
	
	public:
		MeshFromFile(Core& core);

		/**
 		 * Loads model from file.
		 * @param errorState contains the error if the mesh can't be loaded.
		 * @return if the mesh loaded.
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the mesh instance, created during init after a successful load.
		 */
		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }

		/**
		 * @return the mesh instance, created during init after a successful load.
		 */
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

		std::string		mPath;									///< Property: 'Path' path to the '.mesh' file on disk
		EMeshDataUsage	mUsage		= EMeshDataUsage::Static;	///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		ECullMode		mCullMode	= ECullMode::Back;			///< Property: 'CullMode' controls which triangles are culled, back facing, front facing etc.

	private:
		RenderService*						mRenderService = nullptr;
		std::unique_ptr<MeshInstance>		mMeshInstance = nullptr;
	};
}

