#pragma once

#include <QGraphicsScene>
#include <QGraphicsItemGroup>
#include "timelinemodel.h"

namespace napkin {

	class TrackItem;
	class EventItem;

	class TimelineScene : public QGraphicsScene {
	public:
		TimelineScene();
		void setTimeline(Timeline* timeline);
		Timeline* timeline() const { return mTimeline; }
		void setTracksExpanded(const QList<Track*> expandedTracks);

	private:
		void onTrackAdded(Track& track);
		void addTrack(Track& track, TrackItem* parentitem = nullptr);
		void onTrackRemoved(Track& track);
		void onEventAdded(Event& event);
		void onEventRemoved(Event& event);

		QList<TrackItem*> trackItems() const;
		TrackItem* trackItem(Track& track);


		EventItem* eventItem(Event& event);

		Timeline* mTimeline = nullptr;

		QGraphicsItemGroup mTrackGroup;
		QGraphicsItemGroup mEventGroup;
	};
}