#pragma once

#include <QtWidgets/QGraphicsItem>
#include <QtGui/QBrush>
#include <QPen>
#include "timelinemodel.h"

namespace napkin {

	class TrackItem : public QGraphicsRectItem {
	public:
		explicit TrackItem(Track& track);

		Track& track() const { return mTrack; }

	private:
		Track& mTrack;

	};
}