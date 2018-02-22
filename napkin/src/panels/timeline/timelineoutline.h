#pragma once

#include <QVBoxLayout>

#include "generic/filtertreeview.h"
#include "generic/qtutils.h"
#include "timelinemodel.h"
#include "trackitem.h"

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

		void setModel(Timeline* timeline);

		/**
		 * Top of treeview in pixels, relative to this widget (the outline)
		 * @return
		 */
		int getTrackTop() const;

		int getTrackHeight(const Track& track) const;

		void setVerticalScroll(int value);
	Q_SIGNALS:
		void verticalScrollChanged(int value);

	private:
		void onViewResized(const QSize& size);

		QVBoxLayout mLayout;
		FilterTreeView mFilterTree;
		OutlineModel mModel;
	};

} // napkin

