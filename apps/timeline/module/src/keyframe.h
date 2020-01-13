#pragma once

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <fcurve.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 */
	class NAPAPI KeyFrame : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual bool init(utility::ErrorState& errorState) override;

		void adjustValue(const float newValue);

	public:
		float										mValue;
		double										mTime = 0.0;
		std::string									mName = "";
		ResourcePtr<math::FCurve<float, float>>		mCurve;
		ResourcePtr<math::FCurve<float, float>>		mNextCurve;
	};
}
