#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>
#include <rendertarget.h>
#include <polyline.h>
#include <etherdreamdac.h>
#include <visualizenormalsmesh.h>

namespace nap
{
	/**
	 * Holds pointers to laser prototype specific objects
	 */
	class LaserCompound : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		virtual ~LaserCompound();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		// property: Laser Line Mesh
		nap::ObjectPtr<Line> mLineMesh = nullptr;

		// property: Trace Mesh
		nap::ObjectPtr<Line> mTraceMesh = nullptr;

		// property: Normals Mesh
		nap::ObjectPtr<VisualizeNormalsMesh> mNormalsMesh = nullptr;

		// property: Etherdream output DAC
		nap::ObjectPtr<EtherDreamDac> mDac = nullptr;

		// property: Render Target
		nap::ObjectPtr<RenderTarget> mTarget = nullptr;

		// property: Offset
		float mOffset = 0.0f;

		// property: Laser id
		int mLaserID = 0;
	};
}
