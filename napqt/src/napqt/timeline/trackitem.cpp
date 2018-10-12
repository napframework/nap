#include "trackitem.h"

napqt::TrackItem::TrackItem(QGraphicsItem* parent, napqt::Track& track)
		: mTrack(track), QGraphicsItem(parent), mGroupEventItem(this, track) {
	setHandlesChildEvents(false);
	mGroupEventItem.setParentItem(this);
}

QRectF napqt::TrackItem::boundingRect() const
{
	return childrenBoundingRect();
}

void napqt::TrackItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{

}
