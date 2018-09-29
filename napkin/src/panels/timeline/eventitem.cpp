#include "eventitem.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QtGui/QtGui>

napkin::BaseEventItem::BaseEventItem(QGraphicsItem* parent)
		: QObject(), QGraphicsRectItem(parent)
{
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsMovable, true);
	mBrush = QBrush(QColor("#89A"));
	mBrushSelected = QBrush(QColor("#9AB"));

	mPenBorder = QPen(Qt::black, 1);
	mPenBorder.setCosmetic(true);
	mPenBorderSelected = QPen(Qt::white, 1);
	mPenBorderSelected.setCosmetic(true);
}

void napkin::BaseEventItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

QRectF napkin::BaseEventItem::boundingRect() const
{
	qreal penWidth = pen().width();
	return rect();
}

napkin::GroupEventItem::GroupEventItem(QGraphicsItem* parent, napkin::Track& track) : BaseEventItem(parent), mTrack(track)
{
	connect(&track, &Track::changed, [this](Track& track) {
		onTrackOrEventChanged();
	});

	for (auto evt : track.events()) {
		connect(evt, &Event::changed, [this](Event& evt) {
			onTrackOrEventChanged();
		});
	}
	onTrackOrEventChanged();
}

void napkin::GroupEventItem::onTrackOrEventChanged()
{
	qreal start;
	qreal stop;
	bool hasRange = mTrack.range(start, stop) && mTrack.events().isEmpty();
	setVisible(hasRange);
	if (!hasRange)
		return;

	qreal length = stop - start;
	setRect(start, 0, length, mTrack.height());
	update();
}


napkin::EventItem::EventItem(QGraphicsItem* parent, napkin::Event& event) : BaseEventItem(parent), mEvent(event)
{
	event.connect(&event, &Event::changed, this, &EventItem::updateGeometry);
	updateGeometry(event);

}

void napkin::EventItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

void napkin::EventItem::updateGeometry(Event& event)
{
	// update geometry based on event length
	setRect(0, 0, event.length(), event.track().height());
}

