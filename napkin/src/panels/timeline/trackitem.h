#pragma once

#include <QtWidgets/QGraphicsItem>
#include <QtGui/QBrush>
#include <QPen>
#include "timelinemodel.h"
#include "eventitem.h"

namespace napkin {

	class TrackItem : public QGraphicsItem {
	public:
		explicit TrackItem(QGraphicsItem* parent, Track& track);

		QRectF boundingRect() const override;

		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

		Track& track() const { return mTrack; }

	private:
		Track& mTrack;
		GroupEventItem mGroupEventItem;
	};
}