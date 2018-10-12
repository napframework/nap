#include "trackitem.h"

napkin::TrackItem::TrackItem(QGraphicsItem* parent, napkin::Track& track)
		: mTrack(track), QGraphicsItem(parent), mGroupEventItem(this, track) {
	setHandlesChildEvents(false);
	mGroupEventItem.setParentItem(this);
}

QRectF napkin::TrackItem::boundingRect() const
{
	return childrenBoundingRect();
}

void napkin::TrackItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{

}
