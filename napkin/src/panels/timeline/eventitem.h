#pragma once


#include <QGraphicsRectItem>
#include "timelinemodel.h"

namespace napkin {

	class EventItem : public QGraphicsRectItem {
	public:
		explicit EventItem(QGraphicsItem* parent, Event& event);

		Event& event() const { return mClip; }

		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	private:
		Event& mClip;

		QGraphicsTextItem mTextItem;
	};
}