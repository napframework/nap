#include "sequencetracksegmentevent.h"

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentEvent)
	RTTI_PROPERTY("Message", &nap::SequenceTrackSegmentEvent::mMessage, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

nap::SequenceEventPtr nap::SequenceTrackSegmentEvent::createEvent()
{
	return std::make_unique<SequenceEventString>(mMessage);
}