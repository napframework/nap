#include <QtWidgets/QTreeView>
#include <QtGui/QStandardItem>
#include "timelineoutline.h"

using namespace napkin;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OutlineTrackItem
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


OutlineTrackItem::OutlineTrackItem(Track& track) : QGraphicsProxyWidget(), mTrack(track)
{
	setWidget(&mWidget);
	mWidget.setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(new QPushButton("I am a track"));

}

void OutlineTrackItem::setWidth(int width)
{
	mWidget.resize(mWidget.height(), width);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OutlineScene
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OutlineScene::OutlineScene()
{
}

void OutlineScene::setTimeline(Timeline* timeline)
{
	if (mTimeline != nullptr)
	{
		disconnect(mTimeline, &Timeline::trackAdded, this, &OutlineScene::onTrackAdded);
		disconnect(mTimeline, &Timeline::trackRemoved, this, &OutlineScene::onTrackRemoved);
	}
	mTimeline = timeline;
	if (mTimeline != nullptr)
	{
		connect(mTimeline, &Timeline::trackAdded, this, &OutlineScene::onTrackAdded);
		connect(mTimeline, &Timeline::trackRemoved, this, &OutlineScene::onTrackRemoved);
	}

	for (auto track : timeline->tracks())
	{
		onTrackAdded(*track);
	}


}

void OutlineScene::onTrackAdded(Track& track)
{
	auto trackitem = new OutlineTrackItem(track);
	addItem(trackitem);
}

void OutlineScene::onTrackRemoved(Track& track)
{
	removeItem(trackItem(track));
}
OutlineTrackItem* OutlineScene::trackItem(Track& track)
{
	for (auto trackitem : trackItems()) {
		if (&trackitem->track() == &track)
			return trackitem;
	}
	return nullptr;
}

QList<OutlineTrackItem*> OutlineScene::trackItems()
{
	QList<OutlineTrackItem*> ret;
	for (auto item : items()) {
		auto trackitem = dynamic_cast<OutlineTrackItem*>(item);
		if (trackitem != nullptr)
			ret << trackitem;
	}
	return ret;
}

void OutlineScene::resize(const QSize& size)
{
	setSceneRect(0, 0, size.width(), 10000);
	for (auto trackitem : trackItems()) {
		trackitem->setWidth(size.width());
	}
}


void OutlineView::resizeEvent(QResizeEvent* event)
{
	QGraphicsView::resizeEvent(event);
	resized(event->size());
}

OutlineView::OutlineView()
{
	setAlignment(Qt::AlignTop | Qt::AlignLeft);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TimelineOutline
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TimelineOutline::TimelineOutline() : QWidget()
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mView);
	mView.setScene(&mScene);

	connect(&mView, &OutlineView::resized, this, &TimelineOutline::onViewResized);
}

void TimelineOutline::setModel(Timeline* timeline)
{
	mScene.setTimeline(timeline);
}

void TimelineOutline::onViewResized(const QSize& size)
{
	mScene.resize(size);
}
