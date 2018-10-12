#include "eventitem.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QtGui>
#include <QApplication>

napqt::TimelineElementItem::TimelineElementItem(QGraphicsItem* parent) : QGraphicsPathItem(parent)
{
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setAcceptHoverEvents(true);

	mBrush = QBrush(QColor("#89A"));
	mBrushSelected = QBrush(QColor("#9AB"));

	mPenBorder = QPen(Qt::black, 1);
	mPenBorder.setCosmetic(true);
	mPenBorderSelected = QPen(Qt::white, 1);
	mPenBorderSelected.setCosmetic(true);
}


napqt::BaseEventItem::BaseEventItem(QGraphicsItem* parent)
		: TimelineElementItem(parent)
{
}

void napqt::BaseEventItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	// Inverse scale
	auto mtx = painter->matrix();
	qreal sx = mtx.m11();
	qreal sy = mtx.m22();

	painter->setPen(isSelected() ? mPenBorderSelected : mPenBorder);
	painter->setBrush(isSelected() ? mBrushSelected : mBrush);

	painter->save();
	painter->drawRect(rect().adjusted(0, 0, -1 / sx, -1 / sy));
	painter->restore();
}

QRectF napqt::BaseEventItem::boundingRect() const
{
	return rect();
}

void napqt::BaseEventItem::setRange(const napqt::Range& range)
{
	setPos(range.start(), 0);
	setGeometry(QRectF(0, 0, range.length(), rect().height()));
}

void napqt::BaseEventItem::setGeometry(const QRectF& rect)
{
	setRect(rect);
}

napqt::Range napqt::BaseEventItem::range() const
{
	return Range(pos().x(), pos().x() + rect().width());
}

void napqt::BaseEventItem::setRect(const QRectF& rect) {
	mRect = rect;
	QPainterPath p;
	p.addRect(rect);
	setPath(p);
}

napqt::GroupEventItem::GroupEventItem(QGraphicsItem* parent, napqt::Track& track) : BaseEventItem(parent),
																					  mTrack(track)
{
	connect(&track, &Track::changed, [this](Track& track)
	{
		onTrackOrEventChanged();
	});

	for (auto evt : track.events())
	{
		connect(evt, &Event::changed, [this](Event& evt)
		{
			onTrackOrEventChanged();
		});
	}
	onTrackOrEventChanged();
}

void napqt::GroupEventItem::onTrackOrEventChanged()
{
	qreal start;
	qreal stop;
	bool hasRange = mTrack.range(start, stop) && mTrack.events().isEmpty();
	setVisible(hasRange);
	if (!hasRange)
		return;

	qreal length = stop - start;
	setRect(QRectF(start, 0, length, mTrack.height()));
	update();
}


napqt::EventItem::EventItem(QGraphicsItem* parent, napqt::Event& event) : BaseEventItem(parent), mEvent(event)
{
	event.connect(&event, &Event::changed, this, &EventItem::updateGeometryFromEvent);
	updateGeometryFromEvent();

}

void napqt::EventItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	// Inverse scale
	auto mtx = painter->matrix();
	qreal sx = mtx.m11();
	qreal sy = mtx.m22();
	qreal px = mtx.dx();
	qreal py = mtx.dy();
	qreal pw = rect().width() * sx;
	qreal ph = rect().height() * sy;
	QRectF viewRect(px, py, pw, ph);

	painter->setPen(isSelected() ? mPenBorderSelected : mPenBorder);
	painter->setBrush(isSelected() ? mBrushSelected : mBrush);

	painter->save();
	painter->drawRect(rect().adjusted(0, 0, -1 / sx, -1 / sy));
	painter->resetTransform();
	QRectF textRect = viewRect.adjusted(3, 3, -3, -3);
	auto text = painter->fontMetrics().elidedText(mEvent.name(), Qt::ElideRight, textRect.width());
	painter->drawText(textRect, text);
	painter->restore();
}

void napqt::EventItem::updateGeometryFromEvent()
{
	// update geometry based on event length
	setPos(mEvent.start(), pos().y());
	setGeometry(QRectF(0, 0, mEvent.length(), mEvent.track().height()));
}

napqt::TickItem::TickItem(QGraphicsItem* parent, napqt::Tick& tick) : mTick(tick), TimelineElementItem(parent)
{
	qreal size = 10;
	setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setHandlesChildEvents(false);

	QPainterPath path;
	path.moveTo(-size, 0);
	path.lineTo(0, -size);
	path.lineTo(size, 0);
	path.lineTo(0, size);
	path.lineTo(-size, 0);
	setPath(path);

	connect(&tick, &Tick::changed, this, &TickItem::updateGeometryFromEvent);
}
void napqt::TickItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(isSelected() ? mPenBorderSelected : mPenBorder);
	painter->setBrush(isSelected() ? mBrushSelected : mBrush);
	painter->drawPath(path());
}
void napqt::TickItem::updateGeometryFromEvent()
{
	setPos(mTick.time(), pos().y());
}

