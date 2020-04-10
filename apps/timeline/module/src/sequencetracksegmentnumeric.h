#pragma once

// internal includes
#include "sequencetracksegment.h"

// external includes
#include <fcurve.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 */
	class NAPAPI SequenceTrackSegmentNumeric : public SequenceTrackSegment
	{
		RTTI_ENABLE(SequenceTrackSegment)
	public:
		ResourcePtr<math::FCurve<float, float>>		mCurve;
		float										mStartValue;
		float										mEndValue;
	};
}
