// local includes
#include "sequence.h"

RTTI_BEGIN_CLASS(nap::Sequence)
	RTTI_PROPERTY("Sequence Tracks", &nap::Sequence::mTracks, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Duration", &nap::Sequence::mDuration, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
}
