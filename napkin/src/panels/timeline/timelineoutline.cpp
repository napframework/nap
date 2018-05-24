#include "timelineoutline.h"

#include <QScrollBar>


using namespace napkin;

OutlineTrackItem::OutlineTrackItem(Track& track) : QStandardItem(), mTrack(track)
{
	setText(track.name());

	for (auto child : track.childTracks())
		appendRow(new OutlineTrackItem(*child));
}

QVariant OutlineTrackItem::data(int role) const
{
	if (role == Qt::SizeHintRole)
	{
		auto size = QStandardItem::data(role).toSize();
		size.setHeight(track().height());
		return size;
	}
	return QStandardItem::data(role);
}

OutlineModel::OutlineModel() : QStandardItemModel()
{

}

void OutlineModel::setTimeline(Timeline* timeline)
{
	if (mTimeline != nullptr)
	{
		disconnect(mTimeline, &Timeline::trackAdded, this, &OutlineModel::onTrackAdded);
		disconnect(mTimeline, &Timeline::trackRemoved, this, &OutlineModel::onTrackRemoved);
	}
	mTimeline = timeline;
	if (mTimeline != nullptr)
	{
		connect(mTimeline, &Timeline::trackAdded, this, &OutlineModel::onTrackAdded);
		connect(mTimeline, &Timeline::trackRemoved, this, &OutlineModel::onTrackRemoved);
	}

	for (auto track : mTimeline->tracks())
	{
		onTrackAdded(*track);
	}
}

Timeline* OutlineModel::getTimeline() const
{
	return mTimeline;
}


OutlineTrackItem* OutlineModel::trackItem(const Track& track) const
{
	return dynamic_cast<OutlineTrackItem*>(napkin::findItemInModel(*this, [&track](const QStandardItem* item)
	{
		auto trackItem = dynamic_cast<const OutlineTrackItem*>(item);
		if (trackItem == nullptr)
			return false;
		return &trackItem->track() == &track;
	}));
}

void OutlineModel::onTrackAdded(Track& track)
{
	auto trackItem = new OutlineTrackItem(track);
	appendRow(trackItem);
}

void OutlineModel::onTrackRemoved(Track& track)
{

}


TimelineOutline::TimelineOutline() : QWidget()
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mFilterTree);

	auto& tree = mFilterTree.getTreeView();
	tree.setHeaderHidden(true);
//	tree.setAlternatingRowColors(true);
	tree.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	tree.setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

//	connect(tree.verticalScrollBar(), &QScrollBar::valueChanged, this, &TimelineOutline::verticalScrollChanged);

	connect(&tree, &QTreeView::expanded, [this](const QModelIndex& idx)
	{
		trackVisibilityChanged();
	});
	connect(&tree, &QTreeView::collapsed, [this](const QModelIndex& idx)
	{
		trackVisibilityChanged();
	});


	mFilterTree.setModel(&mModel);

}

void TimelineOutline::setTimeline(Timeline* timeline)
{
	mModel.setTimeline(timeline);
}

Timeline* TimelineOutline::getTimeline() const
{
	return mModel.getTimeline();
}


void TimelineOutline::onViewResized(const QSize& size)
{

}

void TimelineOutline::setHeaderHeight(int height)
{
	mFilterTree.getLineEdit().setMinimumHeight(height);
	mFilterTree.getLineEdit().setMaximumHeight(height);
}

int TimelineOutline::getTrackHeight(const Track& track) const
{
	return 0;
}

void TimelineOutline::setVerticalScroll(int value)
{
	mFilterTree.getTreeView().verticalScrollBar()->setValue(value);
}



const QList<Track*> TimelineOutline::getExpandedTracks() const
{
	QList<Track*> tracks;
	getExpandedTracks(tracks);
	return tracks;
}

void TimelineOutline::getExpandedTracks(QList<Track*>& result, const QModelIndex& parent) const
{
	const auto sourcemodel = mFilterTree.getModel();
	const auto& filtermodel = mFilterTree.getFilterModel();

	for (int row = 0, len = filtermodel.rowCount(parent); row < len; row++)
	{
		auto index = filtermodel.index(row, 0, parent);
		auto srcindex = filtermodel.mapToSource(index);

		auto sourceindex = napkin::findIndexInModel(*sourcemodel, [&srcindex](const QModelIndex& idx)
		{
			return idx == srcindex;
		});

		auto trackitem = dynamic_cast<OutlineTrackItem*>(sourcemodel->itemFromIndex(sourceindex));
		assert(trackitem != nullptr);
		result << &trackitem->track();

		if (mFilterTree.getTreeView().isExpanded(index))
			getExpandedTracks(result, index);
	}

}


