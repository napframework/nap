// local includes
#include "sequencetrack.h"

// external includes
#include <nap/resourceptr.h>

RTTI_BEGIN_CLASS(nap::SequenceTrack)
	RTTI_PROPERTY("Segments", &nap::SequenceTrack::mSegments, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Parameter ID", &nap::SequenceTrack::mAssignedParameterID, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

DEFINE_SEQUENCETRACKCURVE(nap::SequenceTrackCurve<float>)
DEFINE_SEQUENCETRACKCURVE(nap::SequenceTrackCurve<glm::vec2>)
DEFINE_SEQUENCETRACKCURVE(nap::SequenceTrackCurve<glm::vec3>)
DEFINE_SEQUENCETRACKCURVE(nap::SequenceTrackCurve<glm::vec4>)

//////////////////////////////////////////////////////////////////////////


namespace nap
{
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
}

template class nap::SequenceTrackCurve<float>;
template class nap::SequenceTrackCurve<glm::vec2>;
template class nap::SequenceTrackCurve<glm::vec3>;
template class nap::SequenceTrackCurve<glm::vec4>;
