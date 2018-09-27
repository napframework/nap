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
	return dynamic_cast<OutlineTrackItem*>(napkin::findItemInModel(*this, [&track](
			const QStandardItem* item)
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

Track* OutlineModel::track(const QModelIndex& idx)
{
	if (!idx.isValid())
		return nullptr;
	auto trackItem = dynamic_cast<OutlineTrackItem*>(itemFromIndex(idx));
	assert(trackItem);
	return &trackItem->track();
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
//	tree.setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

	connect(tree.verticalScrollBar(), &QScrollBar::valueChanged, this, &TimelineOutline::verticalScrollChanged);

	mFilterTree.setModel(&mModel);
	registerTrackVisibilityHandler();
}

void TimelineOutline::registerTrackVisibilityHandler()
{
	auto& tree = mFilterTree.getTreeView();
	connect(&tree, &QTreeView::expanded, [this](const QModelIndex& idx)
	{
		trackVisibilityChanged();
	});
	connect(&tree, &QTreeView::collapsed, [this](const QModelIndex& idx)
	{
		trackVisibilityChanged();
	});
	connect(&mFilterTree.getFilterModel(), &QAbstractItemModel::rowsInserted,
			[this](const QModelIndex& parent, int first, int last)
			{
				trackVisibilityChanged();
			});
	connect(&mFilterTree.getFilterModel(), &QAbstractItemModel::rowsRemoved,
			[this](const QModelIndex& parent, int first, int last)
			{
				trackVisibilityChanged();
			});
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

void TimelineOutline::setVerticalScroll(int value)
{
	mFilterTree.getTreeView().verticalScrollBar()->setValue(value);
}


const QList<Track*> TimelineOutline::getVisibleTracks() const
{
	QList<Track*> tracks;
	getVisibleTracks(tracks);
	return tracks;
}

void TimelineOutline::getVisibleTracks(QList<Track*>& result, const QModelIndex& parent) const
{
	const auto sourcemodel = mFilterTree.getModel();
	const auto& filtermodel = mFilterTree.getFilterModel();

	for (int row = 0, len = filtermodel.rowCount(parent); row < len; row++)
	{
		auto index = filtermodel.index(row, 0, parent);
		auto srcindex = filtermodel.mapToSource(index);

		auto sourceindex = napkin::findIndexInModel(*sourcemodel,
													[&srcindex](const QModelIndex& idx)
													{
														return idx == srcindex;
													});

		auto trackitem = dynamic_cast<OutlineTrackItem*>(sourcemodel->itemFromIndex(sourceindex));
		assert(trackitem != nullptr);
		result << &trackitem->track();

		if (mFilterTree.getTreeView().isExpanded(index))
			getVisibleTracks(result, index);
	}

}

Track* TimelineOutline::track(const QModelIndex& idx)
{
	auto srcIndex = mFilterTree.getFilterModel().mapToSource(idx);
	return mModel.track(srcIndex);
}



