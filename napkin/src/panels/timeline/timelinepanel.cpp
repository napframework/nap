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

	connect(&mOutline, &TimelineOutline::verticalScrollChanged, [this](int value) {
		mView.setVerticalScroll(value);
	});

	int rulerHeight = 30;
	mRuler.setHeight(rulerHeight);
	mOutline.setHeaderHeight(rulerHeight);
	mView.setGridEnabled(false);

	setTimeScale(10);

	demo();
}

void TimelinePanel::setTimeScale(qreal scale)
{
	mView.setTimeScale(scale);
}

void TimelinePanel::showEvent(QShowEvent* event)
{
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
	mRuler.setRange(mView.getViewRange());
}


void TimelinePanel::demo()
{
	namegen::NameGen gen;

	int trackCount = 30;
	int eventCount = 10;

	auto timeline = new Timeline(this);


	{
		auto track = timeline->addTrack("Event Track One", nullptr);
		track->addEvent("FirstEvent", 0, 100);
		track->addEvent("Event Zwei", 100, 150);
		track->addEvent("Derde Event", 160, 250);
	}
	{
		auto track = timeline->addTrack("Second Event Track", nullptr);
		auto child1 = track->addTrack("A Child Track");
		auto child2 = track->addTrack("Another Child Track");
	}
	{
		auto track = timeline->addTrack("Track number Three", nullptr);
		track->addEvent("0", 0, 10);
		track->addEvent("10", 10, 20);
		track->addEvent("20", 20, 30);
		track->addEvent("30", 30, 40);
		track->addEvent("40", 40, 50);
		track->addEvent("50", 50, 60);
		track->addEvent("60", 60, 70);
		track->addEvent("70", 70, 80);
		track->addEvent("80", 80, 90);
		track->addEvent("90", 90, 100);
	}

//	auto framestep = 1.0 / timeline->framerate();
//
//	for (int i = 0; i < trackCount; i++)
//	{
//		auto trackname = QString::fromStdString(gen.multiple());
//		auto track = timeline->addTrack(trackname);
//
//		qreal t = 0;
//		for (int e = 0; e < eventCount; e++)
//		{
//			t += namegen::randint(0, 40) * framestep;
//			qreal len = namegen::randint(20, 300) * framestep;
//			auto eventname = QString::fromStdString(gen.multiple());
//			track->addEvent(eventname, t, t + len);
//			t += len;
//		}
//	}

	setTimeline(timeline);

}







