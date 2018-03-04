#pragma once

#include "timelinemodel.h"
#include "timelineview.h"
#include "timelinescene.h"
#include "gridview.h"
#include "timelineoutline.h"
#include "ruler.h"

#include <cassert>

#include <QPaintEvent>
#include <QPainter>
#include <QtGui>
#include <QGraphicsView>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include <QStyleOptionFrame>
#include <QStylePainter>
#include <QVBoxLayout>
#include <QWidget>
#include <QGraphicsItem>

namespace napkin
{


	/**
	 * Widget with treeview showing tracks and timeline divided by splitter
	 */
	class TimelinePanel : public QWidget
	{
	public:
		TimelinePanel();
		~TimelinePanel() {}

		void setTimeScale(qreal scale);

		void setTimeline(Timeline* timeline);

		void demo();

	protected:
		void showEvent(QShowEvent* event) override;

	private:
		void onTimelineViewTransformed();

		TimelineView mView;
		TimelineScene mScene;

		TimelineOutline mOutline;

		QVBoxLayout mLayout;
		QVBoxLayout mTimelineLayout;
		QWidget mTimelineWidget;
		QSplitter mSplitter;
		Ruler mRuler;
	};

}