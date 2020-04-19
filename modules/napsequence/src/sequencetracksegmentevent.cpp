#include "sequencetracksegmentevent.h"

RTTI_BEGIN_CLASS(nap::SequenceTrackSegmentEvent)
RTTI_PROPERTY("Message", &nap::SequenceTrackSegmentEvent::mMessage, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

nap::SequenceEventPtr nap::SequenceTrackSegmentEvent::createEvent()
{
	std::unique_ptr<SequenceEventString> eventString = std::make_unique<SequenceEventString>();
	eventString->mMessage = mMessage;
	return eventString;
}