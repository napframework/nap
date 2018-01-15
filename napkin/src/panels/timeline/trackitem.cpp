#include "trackitem.h"

napkin::TrackItem::TrackItem(napkin::Track& track) : mTrack(track), QGraphicsRectItem() {
	setBrush(QColor(Qt::gray));
	setPen(Qt::NoPen);
	setRect(0, 0, 1000, track.height());
}
