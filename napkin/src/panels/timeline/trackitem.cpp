#include "trackitem.h"

napkin::TrackItem::TrackItem(QGraphicsItem* parent, napkin::Track& track)
		: mTrack(track), QGraphicsItemGroup(parent) {
	setHandlesChildEvents(false);
}


