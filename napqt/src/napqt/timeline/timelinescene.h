#pragma once

#include <QGraphicsScene>
#include <QGraphicsItemGroup>

#include "timelinemodel.h"
#include "eventitem.h"

namespace napkin
{

	class TrackItem;
	class BaseEventItem;

	class TimelineScene : public QGraphicsScene
	{
	public:
		TimelineScene();
		void setTimeline(Timeline* timeline);
		Timeline* timeline() const { return mTimeline; }
		void setVisibleTracks(const QList<Track*> expandedTracks);
		void setGroupEventsVisible(bool show);
		bool isGroupEventsVisible() const;

		void addTrack(Track& track, Track* parentTrack = nullptr);
		void removeTrack(Track& track);

	private:
		void onEventAdded(Event& event);
		void onEventRemoved(Event& event);
		void onTickAdded(Tick& tick);
		void onTickRemoved(Tick& tick);

		QList<TrackItem*> trackItems() const;
		TrackItem* trackItem(Track& track);



		EventItem* eventItem(Event& event);
		TickItem* tickItem(Tick& tick);

		Event* mGroupEvent = nullptr;

		Timeline* mTimeline = nullptr;
		bool mGroupEventsVisible = true;

		QGraphicsItemGroup mTrackGroup;
		QGraphicsItemGroup mEventGroup;
	};
}