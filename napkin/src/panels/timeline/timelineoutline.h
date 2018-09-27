#pragma once

#include <QVBoxLayout>

#include "generic/filtertreeview.h"
#include "generic/qtutils.h"
#include "timelinemodel.h"

namespace napkin
{
	class OutlineTrackItem : public QStandardItem
	{
	public:
		OutlineTrackItem(Track& track);

		Track& track() const
		{ return mTrack; }

		QVariant data(int role) const override;

	private:
		Track& mTrack;
	};


	class OutlineModel : public QStandardItemModel
	{
	Q_OBJECT
	public:
		OutlineModel();

		void setTimeline(Timeline* timeline);
		Timeline* getTimeline() const;

		OutlineTrackItem* trackItem(const Track& track) const;

	private:
		void onTrackAdded(Track& track);

		void onTrackRemoved(Track& track);

		Timeline* mTimeline = nullptr;
	};


	class TimelineOutline : public QWidget
	{
	Q_OBJECT

	public:
		TimelineOutline();
		~TimelineOutline() {}

		void setTimeline(Timeline* timeline);
		Timeline* getTimeline() const;

		/**
		 * Top of treeview in pixels, relative to this widget (the outline)
		 * @return
		 */
		void setHeaderHeight(int height);
		const QList<Track*> getVisibleTracks() const;
		QAbstractItemModel& treeModel() { return mFilterTree.getFilterModel(); }
		OutlineModel& outlineModel() { return mModel; }
		Track* track(const QModelIndex& idx);
		void setVerticalScroll(int value);

	Q_SIGNALS:
		void verticalScrollChanged(int value);
		void trackVisibilityChanged();


	private:
		void registerTrackVisibilityHandler();
		void onViewResized(const QSize& size);
		void getVisibleTracks(QList<Track*>& result, const QModelIndex& parent = QModelIndex()) const;

		QVBoxLayout mLayout;
		FilterTreeView mFilterTree;
		OutlineModel mModel;
	};

} // napkin

