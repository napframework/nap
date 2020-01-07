// local includes
#include "keyframe.h"

RTTI_BEGIN_CLASS(nap::KeyFrame)
RTTI_PROPERTY("Name", &nap::KeyFrame::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Time", &nap::KeyFrame::mTime, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
}
