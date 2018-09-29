#pragma once


#include "timelinemodel.h"
#include <QGraphicsRectItem>
#include <QtGui/QPen>

namespace napkin
{

	class BaseEventItem : public QObject, public QGraphicsRectItem
	{
	Q_OBJECT
	public:
		explicit BaseEventItem(QGraphicsItem* parent);

		QRectF boundingRect() const override;

		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	protected:
		QPen mPenBorder;
		QPen mPenBorderSelected;
		QBrush mBrush;
		QBrush mBrushSelected;
	};

	class GroupEventItem : public BaseEventItem
	{
	Q_OBJECT
	public:
		GroupEventItem(QGraphicsItem* parent, Track& track);
	private:
		void onTrackOrEventChanged();
		Track& mTrack;
	};

	class EventItem : public BaseEventItem
	{
	Q_OBJECT
	public:
		EventItem(QGraphicsItem* parent, Event& event);

		Event& event() const { return mEvent; }
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	private:
		void updateGeometry(Event& evt);

		Event& mEvent;
	};

}