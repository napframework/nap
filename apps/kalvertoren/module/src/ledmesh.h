#pragma once

// Local Includes
#include "artnetmeshfromfile.h"

// External Includes
#include <meshfromfile.h>
#include <nap/resourceptr.h>
#include <nap/resource.h>

namespace nap
{
	/**
	 * Holds pointers to two meshes that together make up an led mesh
	 */
	class NAPAPI LedMesh : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~LedMesh();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		ResourcePtr<ArtnetMeshFromFile>		mTriangleMesh;		//< Holds the led addressable triangles
		ResourcePtr<MeshFromFile>			mFrameMesh;			//< Holds the mesh that is the frame
	};
}
