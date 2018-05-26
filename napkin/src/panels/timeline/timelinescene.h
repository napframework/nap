#pragma once

#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include "timelinemodel.h"
#include "eventitem.h"

namespace napkin {

	class TrackItem;
	class BaseEventItem;

	class TimelineScene : public QGraphicsScene {
	public:
		TimelineScene();
		void setTimeline(Timeline* timeline);
		Timeline* timeline() const { return mTimeline; }
		void setTracksExpanded(const QList<Track*> expandedTracks);
		void setGroupEventsVisible(bool show);
		bool isGroupEventsVisible() const;

	private:
		void onTrackAdded(Track& track);
		void addTrack(Track& track, TrackItem* parentitem = nullptr);
		void onTrackRemoved(Track& track);
		void onEventAdded(Event& event);
		void onEventRemoved(Event& event);

		BaseEventItem* groupEvent(const Track& track) const;

		QList<TrackItem*> trackItems() const;
		TrackItem* trackItem(Track& track);


		EventItem* eventItem(Event& event);

		Event* mGroupEvent = nullptr;

		Timeline* mTimeline = nullptr;
		bool mGroupEventsVisible = true;

		QGraphicsItemGroup mTrackGroup;
		QGraphicsItemGroup mEventGroup;
	};
}