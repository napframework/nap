#include "timelinepanel.h"

#include <QList>

#include <napqt/randomnames.h>
#include <napqt/qtutils.h>

using namespace napqt;

TimelinePanel::TimelinePanel() : QWidget()
{
	// Main layout
	mLayout = new QVBoxLayout(this);
	setLayout(mLayout);
	mLayout->setContentsMargins(0, 0, 0, 0);
	mLayout->setSpacing(0);

	// TimelineOutline
	mSplitter = new QSplitter(this);
	mSplitter->addWidget(&mOutline);

	// Timeline layout
	mTimelineWidget = new QWidget(mSplitter);
	mTimelineLayout = new QVBoxLayout();
	mTimelineLayout->setSpacing(0);
	mTimelineLayout->setContentsMargins(0, 0, 0, 0);
	mTimelineLayout->addWidget(&mRuler);
	mTimelineLayout->addWidget(&mView);
	mTimelineWidget->setLayout(mTimelineLayout);
	mSplitter->addWidget(mTimelineWidget);

	// Splitter
	mSplitter->setSizes({300, 1000});
	mSplitter->setStretchFactor(0, 0);
	mSplitter->setStretchFactor(1, 1);
	mLayout->addWidget(mSplitter);

	// Data
	mView.setScene(&mScene);
	mView.setPanBounds(QRectF(0, 0, std::numeric_limits<qreal>::max(), std::numeric_limits<qreal>::max()));
	connect(mSplitter, &QSplitter::splitterMoved, this, &TimelinePanel::onTimelineViewTransformed);
	connect(&mView, &GridView::viewTransformed, this, &TimelinePanel::onTimelineViewTransformed);

	connect(&mOutline, &TimelineOutline::verticalScrollChanged, [this](int value) {
		mView.setVerticalScroll(value);
	});

	connect(&mOutline, &TimelineOutline::trackVisibilityChanged, [this]() {
		mScene.setVisibleTracks(mOutline.getVisibleTracks());
		// Constrain scrolling according to outline
		int rectHeight = qMax(0, mOutline.overflowHeight());
		mView.setPanBounds(0, 0, std::numeric_limits<qreal>::infinity(), rectHeight);
	});

	int rulerHeight = 30;
	mRuler.setHeight(rulerHeight);
	mOutline.setHeaderHeight(rulerHeight);
//	mView.setGridEnabled(false);

	setTimeScale(10);

	mScene.setVisibleTracks(mOutline.getVisibleTracks());

	mTimeDisplays.emplace_back(std::make_unique<SMPTEIntervalDisplay>());
	mTimeDisplays.emplace_back(std::make_unique<GeneralTimeIntervalDisplay>());
	mTimeDisplays.emplace_back(std::make_unique<FloatIntervalDisplay>());
	mTimeDisplays.emplace_back(std::make_unique<AnimationIntervalDisplay>());

//    initOutlineModelHandlers();
	createTimeFormatActionGroup();

	setupRulerContextMenu();

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
	mOutline.setTimeline(timeline);
}

void TimelinePanel::onTimelineViewTransformed()
{
	auto scroll = getTranslation(mView.transform());
	int s = qRound(-scroll.y());
	mOutline.setVerticalScroll(s);
	mRuler.setRange(mView.getViewRange());
}

void TimelinePanel::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	onTimelineViewTransformed();
}


void TimelinePanel::demo()
{

}

QActionGroup& TimelinePanel::createTimeFormatActionGroup()
{
	auto actionGroupTimeFormat = new QActionGroup(this);
	for (const auto& timedisplay : mTimeDisplays)
	{
		auto action = actionGroupTimeFormat->addAction(timedisplay->name());
		connect(action, &QAction::triggered, [this, &timedisplay]() {
			mRuler.setDisplayFormat(timedisplay.get());
		});
	}

	return *actionGroupTimeFormat;
}
void TimelinePanel::setupRulerContextMenu()
{
	for (auto& disp : mTimeDisplays)
	{
		auto a = new QAction(disp->name(), &mRuler);
		connect(a, &QAction::triggered, [this, &disp]() {
			mRuler.setDisplayFormat(disp.get());
		});
		mRuler.addAction(a);
	}
	mRuler.setContextMenuPolicy(Qt::ContextMenuPolicy::ActionsContextMenu);
}







