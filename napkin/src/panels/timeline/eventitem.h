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
		bool isResizable() { return true; }
		Range range() const;
		void setRange(const Range& range);
		virtual qreal minLength() const = 0;
		bool hasMinLength() const { return minLength() > 0; }
		virtual qreal maxLength() const = 0;
		bool hasMaxLength() const { return maxLength() > 0; }

	protected:
		void setGeometry(const QRectF& rect);
		QPen mPenBorder;
		QPen mPenBorderSelected;
		QBrush mBrush;
		QBrush mBrushSelected;
		qreal mMinLength = 1;
	};

	class GroupEventItem : public BaseEventItem
	{
	Q_OBJECT
	public:
		GroupEventItem(QGraphicsItem* parent, Track& track);
		qreal minLength() const override { return -1; }
		qreal maxLength() const override { return -1; }
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
		qreal minLength() const override { return mEvent.minLength(); }
		qreal maxLength() const override { return mEvent.maxLength(); };
	private:
		void updateGeometryFromEvent();

		Event& mEvent;
	};

}