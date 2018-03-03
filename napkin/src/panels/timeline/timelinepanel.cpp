#include <nap/logger.h>
#include <generic/randomnames.h>
#include "timelinepanel.h"

using namespace napkin;

TimelinePanel::TimelinePanel() : QWidget()
{
	// Main layout
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(0);

	// TimelineOutline
	mSplitter.addWidget(&mOutline);

	// Timeline layout
	mTimelineLayout.setSpacing(0);
	mTimelineLayout.setContentsMargins(0, 0, 0, 0);
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

	connect(&mOutline, &TimelineOutline::verticalScrollChanged, [this](int value) {
		mView.setVerticalScroll(value);
	});

	demo();
}

void TimelinePanel::showEvent(QShowEvent* event)
{
	mView.setTopMargin(mOutline.getTrackTop());

}


void TimelinePanel::setTimeline(Timeline* timeline)
{
	mScene.setTimeline(timeline);
	mOutline.setModel(timeline);
}

void TimelinePanel::onTimelineViewTransformed()
{
	auto scroll = getTranslation(mView.transform());
	int s = qRound(-scroll.y());
	mOutline.setVerticalScroll(s);
}


void TimelinePanel::demo()
{
	namegen::NameGen gen;

	int trackCount = 30;
	int eventCount = 10;

	auto timeline = new Timeline(this);
	auto framestep = 1.0 / timeline->framerate();

	for (int i = 0; i < trackCount; i++)
	{
		auto trackname = QString::fromStdString(gen.multiple());
		auto track = timeline->addTrack(trackname);

		qreal t = 0;
		for (int e = 0; e < eventCount; e++)
		{
			t += namegen::randint(0, 40) * framestep;
			qreal len = namegen::randint(20, 300) * framestep;
			auto eventname = QString::fromStdString(gen.multiple());
			track->addEvent(eventname, t, t + len);
			t += len;
		}
	}

	setTimeline(timeline);

}






