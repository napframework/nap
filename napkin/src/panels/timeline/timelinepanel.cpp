#include <nap/logger.h>
#include <generic/randomnames.h>
#include "timelinepanel.h"

using namespace napkin;

RulerWidget::RulerWidget() {
	setMinimumHeight(20);
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


TLOutlineItem::TLOutlineItem(Track& track, QWidget* parent) : mTrack(track), QWidget(parent) {
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

void TimelineOutline::setModel(Timeline* timeline) {
	if (mTimeline) {
		disconnect(mTimeline, &Timeline::trackAdded, this, &TimelineOutline::onTrackAdded);
		disconnect(mTimeline, &Timeline::trackRemoved, this, &TimelineOutline::onTrackRemoved);
	}

	mTimeline = timeline;

	for (auto track : mTimeline->tracks())
		onTrackAdded(*track);

	connect(mTimeline, &Timeline::trackAdded, this, &TimelineOutline::onTrackAdded);
	connect(mTimeline, &Timeline::trackRemoved, this, &TimelineOutline::onTrackRemoved);
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

void TimelineOutline::onTrackAdded(Track& track) {
	auto widget = new TLOutlineItem(track, this);
	mLayout.insertWidget(mLayout.count() - 1, widget);
	mTracks << widget;
}

void TimelineOutline::onTrackRemoved(Track& track) {
	mLayout.removeWidget(widget(track));
}

TLOutlineItem* TimelineOutline::widget(Track& track) {
	for (auto widget : mTracks)
		if (&widget->track() == &track)
			return widget;
	return nullptr;
}


TimelinePanel::TimelinePanel() : QWidget() {
	// Main layout
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);

	// Outline
	mSplitter.addWidget(&mOutline);

	// Timeline layout
	mTimelineLayout.setSpacing(0);
	mTimelineLayout.setContentsMargins(0, 0, 0, 0);
	mTimelineLayout.addWidget(&mRuler);
	mTimelineLayout.addWidget(&mView);
	mTimelineWidget.setLayout(&mTimelineLayout);
	mSplitter.addWidget(&mTimelineWidget);

	// Splitter
	mSplitter.setSizes({300, 1000});
	mSplitter.setStretchFactor(0, 0);
	mSplitter.setStretchFactor(1, 1);
	mLayout.addWidget(&mSplitter);


	// Data
	mView.setScene(&mScene);
	connect(&mView, &GridView::viewTransformed, this, &TimelinePanel::onTimelineViewTransformed);

	setHeaderHeight(20);

	demo();
}

TimelinePanel::~TimelinePanel() {

}


void TimelinePanel::setTimeline(Timeline* timeline) {
	mScene.setModel(timeline);
	mOutline.setModel(timeline);
}

void TimelinePanel::onTimelineViewTransformed(const QTransform& transform) {

}


void TimelinePanel::demo() {
	namegen::NameGen gen;

	int trackCount = 6;
	int eventCount = 10;

	auto timeline = new Timeline(this);
	auto framestep = 1.0 / timeline->framerate();

	for (int i = 0; i < trackCount; i++) {
		auto trackname = QString::fromStdString(gen.multiple());
		auto track = timeline->addTrack(trackname);

		qreal t = 0;
		for (int e = 0; e < eventCount; e++) {
			t += namegen::randint(0, 40) * framestep;
			qreal len = namegen::randint(20, 300) * framestep;
			auto eventname = QString::fromStdString(gen.multiple());
			track->addEvent(eventname, t, t + len);
			t += len;
		}
	}

	setTimeline(timeline);

}

void TimelinePanel::setHeaderHeight(int height) {
	mRuler.setMinimumHeight(height);
	mRuler.setMaximumHeight(height);
	mOutline.setHeaderHeight(height);
}





