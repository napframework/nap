#include <nap/logger.h>
#include <generic/randomnames.h>
#include "timelinepanel.h"

void ViewTransform::translate(QPointF delta) {
	pos = pos + delta;
}

void ViewTransform::translate(qreal x, qreal y) {
	translate(QPointF(x, y));
}

void ViewTransform::zoom(QPointF delta) {
	scale.setX(scale.x() * delta.x());
	scale.setY(scale.y() * delta.y());
}


RulerWidget::RulerWidget() {
	setMinimumHeight(20);
}

void RulerWidget::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QStylePainter ptr;
	QStyleOptionFrame op;
	op.initFrom(this);

	qreal scale = mViewTransform.scale.x();
	qreal spacing = 50 * scale;

	ptr.begin(this);
	{
		ptr.drawPrimitive(QStyle::PE_PanelButtonBevel, op);

		ptr.setPen(QPen(Qt::black, 0));
		int steps = qCeil(event->rect().width() / spacing) + 1;
		int top = 0;
		int off = qRound(fmod(mViewTransform.pos.x(), spacing));
		int bottom = event->rect().height();
		for (int i = 0; i < steps; i++) {
			int x = qRound(off + i * spacing);
			ptr.drawLine(x, top, x, bottom);
		}
	}
	ptr.end();
}

void RulerWidget::setViewTransform(const ViewTransform& xf) {
	mViewTransform = xf;
	update();
}


void OutlineHeader::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QStylePainter ptr;
	QStyleOptionFrame op;
	op.initFrom(this);

	int spacing = 50;

	ptr.begin(this);
	{
		ptr.drawPrimitive(QStyle::PE_PanelButtonBevel, op);
	}
	ptr.end();
}

void TLTrackWidget::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QStylePainter ptr;
	QStyleOptionFrame op;
	op.initFrom(this);

	int spacing = 50;

	ptr.begin(this);
	{
		ptr.fillRect(event->rect(), Qt::gray);
	}
	ptr.end();
}

void TLEventWidget::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QStylePainter ptr;
	QStyleOptionFrame op;
	op.initFrom(this);

	int spacing = 50;

	ptr.begin(this);
	{
		ptr.setPen(Qt::black);
		ptr.setBrush(mEvent.color());
		ptr.drawRect(rect().adjusted(0, 0, -1, -1));
		ptr.drawText(rect(), mEvent.name());
	}
	ptr.end();
}

void TLEventWidget::mousePressEvent(QMouseEvent* evt) {
	QWidget::mousePressEvent(evt);
}

TLOutlineItem::TLOutlineItem(TLTrack& track, QWidget* parent) : mTrack(track), QWidget(parent) {
	setLayout(&mLayout);
	mLayout.setSpacing(0);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mLabel);
	mLabel.setText(track.name());

	setHeight(track.height());
}

void TLOutlineItem::setHeight(int height) {
	setMinimumHeight(height);
	setMaximumHeight(height);
}

void TLOutlineItem::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QStylePainter ptr;
	QStyleOptionFrame op;
	op.initFrom(this);

	ptr.begin(this);
	{
		ptr.drawPrimitive(QStyle::PE_PanelButtonBevel, op);
	}
	ptr.end();
}

TimelineOutline::TimelineOutline() : mHolder(this) {
	mHolder.setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);
	mLayout.addWidget(&mHeader);
	mLayout.addStretch(1);
}

void TimelineOutline::setModel(TLTimeline* timeline) {
	if (mTimeline) {
		disconnect(mTimeline, &TLTimeline::trackAdded, this, &TimelineOutline::onTrackAdded);
		disconnect(mTimeline, &TLTimeline::trackRemoved, this, &TimelineOutline::onTrackRemoved);
	}

	mTimeline = timeline;

	for (auto track : mTimeline->tracks())
		onTrackAdded(*track);

	connect(mTimeline, &TLTimeline::trackAdded, this, &TimelineOutline::onTrackAdded);
	connect(mTimeline, &TLTimeline::trackRemoved, this, &TimelineOutline::onTrackRemoved);
}

void TimelineOutline::setHeaderHeight(int height) {
	mHeader.setMinimumHeight(height);
	mHeader.setMaximumHeight(height);
}

void TimelineOutline::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);

	QPainter ptr;
	ptr.begin(this);
	ptr.fillRect(event->rect(), Qt::darkGray);
	ptr.end();
}

void TimelineOutline::onTrackAdded(TLTrack& track) {
	auto widget = new TLOutlineItem(track, this);
	mLayout.insertWidget(mLayout.count() - 1, widget);
	mTracks << widget;
}

void TimelineOutline::onTrackRemoved(TLTrack& track) {
	mLayout.removeWidget(widget(track));
}

TLOutlineItem* TimelineOutline::widget(TLTrack& track) {
	for (auto widget : mTracks)
		if (&widget->track() == &track)
			return widget;
	return nullptr;
}

void TimelineOutline::setViewTransform(const ViewTransform& xf) {
	auto rect = mHolder.childrenRect();
	mHolder.setGeometry(0, xf.pos.y(), rect.width(), rect.height());
	update();
}

void TimelineCanvas::setModel(TLTimeline* timeline) {
	if (mTimeline) {
		disconnect(mTimeline, &TLTimeline::trackAdded, this, &TimelineCanvas::onTrackAdded);
		disconnect(mTimeline, &TLTimeline::trackRemoved, this, &TimelineCanvas::onTrackRemoved);
	}

	mTimeline = timeline;

	for (auto track : mTimeline->tracks())
		onTrackAdded(*track);

	connect(mTimeline, &TLTimeline::trackAdded, this, &TimelineCanvas::onTrackAdded);
	connect(mTimeline, &TLTimeline::trackRemoved, this, &TimelineCanvas::onTrackRemoved);
}

void TimelineCanvas::resizeEvent(QResizeEvent* event) {
	QWidget::resizeEvent(event);
	invalidateLayout();
}

void TimelineCanvas::wheelEvent(QWheelEvent* event) {
	QWidget::wheelEvent(event);
}

void TimelineCanvas::paintEvent(QPaintEvent* event) {
	layout();

	QWidget::paintEvent(event);

	QPainter ptr;
	ptr.begin(this);
	ptr.fillRect(event->rect(), Qt::darkGray);

	ptr.end();

}

void TimelineCanvas::clear() {
	for (auto track : mTimeline->tracks())
		onTrackRemoved(*track);
}

void TimelineCanvas::onTrackAdded(TLTrack& track) {
	new TLTrackWidget(&mTrackHolder, track);

	for (auto event : track.events())
		onEventAdded(*event);

	connect(&track, &TLTrack::eventAdded, this, &TimelineCanvas::onEventAdded);
	connect(&track, &TLTrack::eventRemoved, this, &TimelineCanvas::onEventRemoved);

	invalidateLayout();
}

void TimelineCanvas::onTrackRemoved(TLTrack& track) {
	disconnect(&track, &TLTrack::eventAdded, this, &TimelineCanvas::onEventAdded);
	disconnect(&track, &TLTrack::eventRemoved, this, &TimelineCanvas::onEventRemoved);

	delete trackWidget(track);

	invalidateLayout();
}

void TimelineCanvas::onEventAdded(TLEvent& event) {
	new TLEventWidget(&mEventHolder, event);

	invalidateLayout();
}

void TimelineCanvas::onEventRemoved(TLEvent& event) {
	delete eventWidget(event);

	invalidateLayout();
}

void TimelineCanvas::invalidateLayout() {
	mLayoutValid = false;
}

void TimelineCanvas::layout() {
	if (mLayoutValid)
		return;

	int y = 0;

	auto topLeft = mViewTransform.pos.toPoint();
	mTrackHolder.setGeometry(0, topLeft.y(), width(), height());
	qreal scale = mViewTransform.scale.x();

	for (auto trackWidget : trackWidgets()) {
		auto& track = trackWidget->track();

		trackWidget->setGeometry(0, y, width(), track.height());

		for (auto event : track.events()) {
			auto w = eventWidget(*event);
			int x = qRound(event->start() * scale);
			int width = qRound((event->end() - event->start())* scale);

			w->setGeometry(x, y, width, track.height());
		}

		y += track.height();
	}
	mEventHolder.setGeometry(QRect(topLeft, mEventHolder.childrenRect().size()));

	mLayoutValid = true;
}

TimelineCanvas::TimelineCanvas()
		: mTrackHolder(this), mEventHolder(this), QWidget() {
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	setMouseTracking(true);

}

const QList<TLTrackWidget*> TimelineCanvas::trackWidgets() {
	QList<TLTrackWidget*> ret;
	for (auto child : mTrackHolder.children()) {
		auto trackWidget = dynamic_cast<TLTrackWidget*>(child);
		assert(trackWidget);
		ret << trackWidget;
	}
	return ret;
}

const QList<TLEventWidget*> TimelineCanvas::eventWidgets() {
	QList<TLEventWidget*> ret;
	for (auto child : mEventHolder.children()) {
		auto eventWidget = dynamic_cast<TLEventWidget*>(child);
		assert(eventWidget);
		ret << eventWidget;
	}
	return ret;
}

TLTrackWidget* TimelineCanvas::trackWidget(TLTrack& track) {
	for (auto w : trackWidgets())
		if (&w->track() == &track)
			return w;
	return nullptr;
}

TLEventWidget* TimelineCanvas::eventWidget(TLEvent& event) {
	for (auto w : eventWidgets())
		if (&w->event() == &event)
			return w;
	return nullptr;
}

void TimelineCanvas::mousePressEvent(QMouseEvent* event) {
	QWidget::mousePressEvent(event);
	mMousePressPos = event->pos();
	mMouseLastPos = event->pos();
}

void TimelineCanvas::mouseMoveEvent(QMouseEvent* event) {
	QWidget::mouseMoveEvent(event);
	auto mousePos = event->pos();
	auto delta = mousePos - mMouseLastPos;

	bool lmb = event->buttons() == Qt::LeftButton;
	bool rmb = event->buttons() == Qt::RightButton;
	bool altKey = event->modifiers() == Qt::AltModifier;

	if (lmb && altKey) {
		pan(QPointF(delta));
	} else if (altKey && rmb) {
		zoom(QPointF(1, 1) + QPointF(delta) * 0.01);
	}

	mMouseLastPos = mousePos;
}

void TimelineCanvas::mouseReleaseEvent(QMouseEvent* event) {
	QWidget::mouseReleaseEvent(event);
}

void TimelineCanvas::keyPressEvent(QKeyEvent* event) {
	QWidget::keyPressEvent(event);
}

void TimelineCanvas::keyReleaseEvent(QKeyEvent* event) {
	QWidget::keyReleaseEvent(event);
}

void TimelineCanvas::zoom(const QPointF& delta) {
	qreal x = mViewTransform.scale.x() * delta.x();
	mViewTransform.scale.setX(x);
	invalidateLayout();
	update();
	viewTransformed(mViewTransform);
}

void TimelineCanvas::pan(const QPointF& delta) {
	qreal x = std::fmin(mViewTransform.pos.x() + delta.x(), 0);
	qreal y = std::fmin(mViewTransform.pos.y() + delta.y(), 0);
	mViewTransform.pos.setX(x);
	mViewTransform.pos.setY(y);
	invalidateLayout();
	update();
	viewTransformed(mViewTransform);
}

void TimelineView::setModel(TLTimeline* timeline) {
	mCanvas.setModel(timeline);
}

void TimelineView::setHeaderHeight(int height) {
	mRuler.setMinimumHeight(height);
	mRuler.setMaximumHeight(height);
}

TimelineView::TimelineView() : QWidget() {
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);
	mLayout.addWidget(&mRuler);
	mLayout.addWidget(&mCanvas);

	connect(&mCanvas, &TimelineCanvas::viewTransformed, this, &TimelineView::onTimelineTransformed);
}

void TimelineView::onTimelineTransformed(const ViewTransform& xf) {
	mRuler.setViewTransform(xf);
	timelineTransformChanged(xf);
}

TimelinePanel::TimelinePanel() : QWidget() {
	setLayout(&mLayout);
	mLayout.setSpacing(0);
	mLayout.addWidget(&mSplitter);
	mSplitter.addWidget(&mOutline);
	mSplitter.addWidget(&mTimeline);


	mSplitter.setSizes({300, 100});
	mSplitter.setStretchFactor(0, 0);
	mSplitter.setStretchFactor(1, 1);

	setHeaderHeight(20);

	demo();
}

void TimelinePanel::setTimeline(TLTimeline* timeline) {
	mTimeline.setModel(timeline);
	mOutline.setModel(timeline);
}

void TimelinePanel::demo() {
	namegen::NameGen gen;

	int trackCount = 100;
	int eventCount = 10;

	auto timeline = new TLTimeline(this);

	for (int i=0; i <trackCount; i++){
		auto trackname = QString::fromStdString(gen.multiple());
		auto track = timeline->addTrack(trackname);

		int t = 0;
		for (int e=0; e < eventCount; e++) {
			t += namegen::randint(0, 40);
			int len = namegen::randint(20, 300);
			auto eventname = QString::fromStdString(gen.multiple());
			track->addEvent(eventname, t, t + len);
			t += len;
		}
	}

	setTimeline(timeline);

}

void TimelinePanel::setHeaderHeight(int height) {
	mTimeline.setHeaderHeight(height);
	mOutline.setHeaderHeight(height);
}

void TimelinePanel::onTimelineTransformChanged(const ViewTransform& xf) {
	mOutline.setViewTransform(xf);
}

