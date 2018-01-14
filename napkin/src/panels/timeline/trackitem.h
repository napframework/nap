#pragma once

#include <QtWidgets/QGraphicsItem>
#include "timelinemodel.h"

namespace napkin {

	class TrackItem : public QGraphicsRectItem {
	public:
		explicit TrackItem(Track& track) : mTrack(track), QGraphicsRectItem() {}

		Track& track() const { return mTrack; }

	private:
		Track& mTrack;

	};
}