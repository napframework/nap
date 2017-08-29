#pragma once

#include "mesh.h"

namespace nap
{
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

}

