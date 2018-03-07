#pragma once

// Local Includes
#include "artnetmeshfromfile.h"

// External Includes
#include <rtti/rttiobject.h>
#include <meshfromfile.h>

namespace nap
{
	/**
	 * Holds pointers to two meshes that together make up an led mesh
	 */
	class NAPAPI LedMesh : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		virtual ~LedMesh();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		rtti::ObjectPtr<ArtnetMeshFromFile>	mTriangleMesh;		//< Holds the led addressable triangles
		rtti::ObjectPtr<MeshFromFile>			mFrameMesh;			//< Holds the mesh that is the frame
	};
}
