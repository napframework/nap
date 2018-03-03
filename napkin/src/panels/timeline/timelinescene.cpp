#include "timelinescene.h"
#include "eventitem.h"
#include "trackitem.h"

using namespace napkin;

TimelineScene::TimelineScene() {
	qreal extent = 100000;
	setSceneRect(-extent, -extent, extent*2, extent*2);

	mTrackGroup.setHandlesChildEvents(false);
	mEventGroup.setHandlesChildEvents(false);

	addItem(&mTrackGroup);
	addItem(&mEventGroup);
}

void TimelineScene::setTimeline(Timeline* timeline) {
	if (mTimeline) {
		disconnect(mTimeline, &Timeline::trackAdded, this, &TimelineScene::onTrackAdded);
		disconnect(mTimeline, &Timeline::trackRemoved, this, &TimelineScene::onTrackRemoved);
	}

	mTimeline = timeline;
	mTimeline->setParent(this);

	for (auto track : mTimeline->tracks())
		onTrackAdded(*track);

	connect(mTimeline, &Timeline::trackAdded, this, &TimelineScene::onTrackAdded);
	connect(mTimeline, &Timeline::trackRemoved, this, &TimelineScene::onTrackRemoved);
}


void TimelineScene::onTrackAdded(Track& track) {
	auto item = new TrackItem(&mTrackGroup, track);

	item->setY(item->track().height() * item->track().index());

	for (auto event : track.events())
		onEventAdded(*event);

	connect(&track, &Track::eventAdded, this, &TimelineScene::onEventAdded);
	connect(&track, &Track::eventRemoved, this, &TimelineScene::onEventRemoved);

}

void TimelineScene::onTrackRemoved(Track& track) {
	disconnect(&track, &Track::eventAdded, this, &TimelineScene::onEventAdded);
	disconnect(&track, &Track::eventRemoved, this, &TimelineScene::onEventRemoved);

	auto item = trackItem(track);
	mTrackGroup.removeFromGroup(item);
	removeItem(item);
}

void TimelineScene::onEventAdded(Event& event) {
	auto item = new EventItem(&mEventGroup, event);

	item->setX(event.start());
	item->setY(event.track().index() * event.track().height());
}

void TimelineScene::onEventRemoved(Event& event) {
	auto item = eventItem(event);
	mEventGroup.removeFromGroup(item);
	removeItem(item);
}


TrackItem* TimelineScene::trackItem(Track& track) {
	for (auto item : mTrackGroup.childItems()) {
		auto trackItem = dynamic_cast<TrackItem*>(item);
		if (&trackItem->track() == &track)
			return trackItem;
	}
	return nullptr;
}

EventItem* TimelineScene::eventItem(Event& event) {
	for (auto item : mEventGroup.childItems()) {
		auto eventItem = dynamic_cast<EventItem*>(item);
		if (&eventItem->event() == &event)
			return eventItem;
	}
	return nullptr;
}