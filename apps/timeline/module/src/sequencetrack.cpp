// local includes
#include "sequencetrack.h"
#include "sequencetracksegmentcurve.h"

// external includes
#include <nap/resourceptr.h>

RTTI_BEGIN_CLASS(nap::SequenceTrack)
	RTTI_PROPERTY("Segments", &nap::SequenceTrack::mSegments, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Parameter ID", &nap::SequenceTrack::mAssignedObjectIDs, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequenceTrackEvent)
RTTI_END_CLASS

DEFINE_SEQUENCETRACKCURVE(nap::SequenceTrackCurve<float>)
DEFINE_SEQUENCETRACKCURVE(nap::SequenceTrackCurve<glm::vec2>)
DEFINE_SEQUENCETRACKCURVE(nap::SequenceTrackCurve<glm::vec3>)
DEFINE_SEQUENCETRACKCURVE(nap::SequenceTrackCurve<glm::vec4>)

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	template<typename T>
	bool SequenceTrackCurve<T>::init(utility::ErrorState& errorState)
	{
		if (SequenceTrack::init(errorState))
		{
			for (int i = 0; i < mSegments.size(); i++)
			{
				if (!errorState.check(mSegments[i].get()->get_type().is_derived_from<SequenceTrackSegmentCurve<T>>(), "segment not derived from correct type"))
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}

		return true;
	}

	template<>
	const SequenceTrackTypes::Types SequenceTrackCurve<float>::getTrackType() const
	{
		return SequenceTrackTypes::FLOAT;
	}

	template<>
	const SequenceTrackTypes::Types SequenceTrackCurve<glm::vec2>::getTrackType() const
	{
		return SequenceTrackTypes::VEC2;
	}

	template<>
	const SequenceTrackTypes::Types SequenceTrackCurve<glm::vec3>::getTrackType() const
	{
		return SequenceTrackTypes::VEC3;
	}

	template<>
	const SequenceTrackTypes::Types SequenceTrackCurve<glm::vec4>::getTrackType() const
	{
		return SequenceTrackTypes::VEC4;
	}

	const SequenceTrackTypes::Types SequenceTrackEvent::getTrackType() const
	{
		return SequenceTrackTypes::EVENT;
	}
}

template class nap::SequenceTrackCurve<float>;
template class nap::SequenceTrackCurve<glm::vec2>;
template class nap::SequenceTrackCurve<glm::vec3>;
template class nap::SequenceTrackCurve<glm::vec4>;
