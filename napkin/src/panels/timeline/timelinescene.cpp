#include "timelinescene.h"
#include "eventitem.h"
#include "trackitem.h"

#include <QList>

using namespace napkin;

TimelineScene::TimelineScene()
{
	qreal extent = 100000;
	setSceneRect(-extent, -extent, extent * 2, extent * 2);

	mTrackGroup.setHandlesChildEvents(false);
	mEventGroup.setHandlesChildEvents(false);

	addItem(&mTrackGroup);
	addItem(&mEventGroup);
}

void TimelineScene::setTimeline(Timeline* timeline)
{
	if (mTimeline)
	{
//        disconnect(mTimeline, &Timeline::trackAdded, this, &TimelineScene::onTrackAdded);
//        disconnect(mTimeline, &Timeline::trackRemoved, this, &TimelineScene::removeTrack);
	}

	mTimeline = timeline;
	mTimeline->setParent(this);

	for (auto track : mTimeline->tracks())
		addTrack(*track);

//    connect(mTimeline, &Timeline::trackAdded, this, &TimelineScene::onTrackAdded);
//    connect(mTimeline, &Timeline::trackRemoved, this, &TimelineScene::removeTrack);
}


void TimelineScene::addTrack(Track& track, Track* parentTrack)
{
	TrackItem* parentTrackItem = nullptr;
	if (parentTrack)
		parentTrackItem = trackItem(*parentTrack);

	TrackItem* item = nullptr;
	if (parentTrackItem)
		item = new TrackItem(parentTrackItem, track);
	else
		item = new TrackItem(&mTrackGroup, track);

	item->setY(item->track().height() * item->track().index());

	for (auto event : track.events())
		onEventAdded(*event);

	for (auto tick : track.ticks())
		onTickAdded(*tick);

	for (auto childtrack : track.childTracks())
		addTrack(*childtrack);

	connect(&track, &Track::eventAdded, this, &TimelineScene::onEventAdded);
	connect(&track, &Track::eventRemoved, this, &TimelineScene::onEventRemoved);
	connect(&track, &Track::tickAdded, this, &TimelineScene::onTickAdded);
	connect(&track, &Track::tickRemoved, this, &TimelineScene::onTickRemoved);

}

void TimelineScene::removeTrack(Track& track)
{
	disconnect(&track, &Track::eventAdded, this, &TimelineScene::onEventAdded);
	disconnect(&track, &Track::eventRemoved, this, &TimelineScene::onEventRemoved);

	auto item = trackItem(track);
	mTrackGroup.removeFromGroup(item);
	removeItem(item);
}

void TimelineScene::onEventAdded(Event& event)
{
	auto item = new EventItem(trackItem(event.track()), event);

	item->setX(event.start());
//	item->setY(event.track().index() * event.track().height());
}

void TimelineScene::onEventRemoved(Event& event)
{
	auto item = eventItem(event);
	item->setParent(nullptr);
	removeItem(item);
}

void TimelineScene::onTickAdded(Tick& tick)
{
	auto& track = tick.track();
	auto item = new TickItem(trackItem(track), tick);
	item->setX(tick.time());
	item->setY(track.height() / 2);
}

void TimelineScene::onTickRemoved(Tick& tick)
{
	auto item = tickItem(tick);
	item->setParentItem(nullptr);
	removeItem(item);
}


QList<TrackItem*> TimelineScene::trackItems() const
{
	QList<TrackItem*> items;
	for (auto item : mTrackGroup.childItems())
	{
		auto trackItem = dynamic_cast<TrackItem*>(item);
		if (nullptr != trackItem)
			items << trackItem;
	}
	return items;
}

TrackItem* TimelineScene::trackItem(Track& track)
{
	for (auto item : mTrackGroup.childItems())
	{
		auto trackItem = dynamic_cast<TrackItem*>(item);
		if (&trackItem->track() == &track)
			return trackItem;
	}
	return nullptr;
}

EventItem* TimelineScene::eventItem(Event& event)
{
	for (auto item : mEventGroup.childItems())
	{
		auto eventItem = dynamic_cast<EventItem*>(item);
		if (eventItem && &eventItem->event() == &event)
			return eventItem;
	}
	return nullptr;
}

TickItem* TimelineScene::tickItem(Tick& tick)
{
	for (auto item : mEventGroup.childItems())
	{
		auto tickItem = dynamic_cast<TickItem*>(item);
		if (tickItem && &tickItem->tick() == &tick)
			return tickItem;
	}
	return nullptr;
}


void TimelineScene::setVisibleTracks(const QList<Track*> expandedTracks)
{
	qreal y = 0;

	for (auto item : trackItems())
	{
		bool visible = expandedTracks.contains(&item->track());
		item->setVisible(visible);
		if (visible)
		{
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

