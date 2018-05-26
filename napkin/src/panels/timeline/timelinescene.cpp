#include "timelinescene.h"
#include "eventitem.h"
#include "trackitem.h"

#include <QList>

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
		addTrack(*track);

	connect(mTimeline, &Timeline::trackAdded, this, &TimelineScene::onTrackAdded);
	connect(mTimeline, &Timeline::trackRemoved, this, &TimelineScene::onTrackRemoved);
}

void TimelineScene::onTrackAdded(Track& track)
{
	addTrack(track, nullptr);
}

void TimelineScene::addTrack(Track& track, TrackItem* parentitem) {

	TrackItem* item = nullptr;
	if (parentitem)
		item = new TrackItem(parentitem, track);
	else
		item = new TrackItem(&mTrackGroup, track);

	item->setY(item->track().height() * item->track().index());

	for (auto event : track.events())
		onEventAdded(*event);

	for (auto childtrack : track.childTracks())
		addTrack(*childtrack);

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
	auto item = new EventItem(trackItem(event.track()), event);

	item->setX(event.start());
//	item->setY(event.track().index() * event.track().height());
}

void TimelineScene::onEventRemoved(Event& event) {
	auto item = eventItem(event);
	item->setParent(nullptr);
	removeItem(item);
}

QList<TrackItem*> TimelineScene::trackItems() const
{
	QList<TrackItem*> items;
	for (auto item : mTrackGroup.childItems()) {
		auto trackItem = dynamic_cast<TrackItem*>(item);
		if (nullptr != trackItem)
			items << trackItem;
	}
	return items;
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
		if (eventItem && &eventItem->event() == &event)
			return eventItem;
	}
	return nullptr;
}

void TimelineScene::setTracksExpanded(const QList<Track*> expandedTracks)
{
	qreal y = 0;

	for (auto item : trackItems()) {
		bool visible = expandedTracks.contains(&item->track());
		item->setVisible(visible);
		if (visible) {
			item->setY(y);
			y += item->track().height();
		}
	}
}

void TimelineScene::setGroupEventsVisible(bool show)
{
	mGroupEventsVisible = show;
}

bool TimelineScene::isGroupEventsVisible() const
{
	return mGroupEventsVisible;
}

BaseEventItem* TimelineScene::groupEvent(const Track& track) const
{
	if (!mGroupEventsVisible)
		return nullptr;

//	BaseEventItem* eventItem = ;
}


