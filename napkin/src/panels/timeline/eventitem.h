#pragma once


#include <QGraphicsRectItem>
#include <QtGui/QPen>
#include "timelinemodel.h"

namespace napkin {

	class EventItem : public QObject, public QGraphicsRectItem {
		Q_OBJECT
	public:
		explicit EventItem(QGraphicsItem* parent, Event& event);

		Event& event() const { return mEvent; }

		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	private:
		void onEventChanged(Event& evt);

		Event& mEvent;
		QPen mPenBorder;
		QPen mPenBorderSelected;
		QBrush mBrush;
		QBrush mBrushSelected;
	};
}