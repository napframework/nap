#pragma once

#include "timelinemodel.h"
#include "timelineview.h"
#include "timelinescene.h"
#include "gridview.h"
#include "timelineoutline.h"

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



	class TimelinePanel : public QWidget
	{
	public:
		TimelinePanel();

		~TimelinePanel() override;

		void setTimeline(Timeline* timeline);

		void setHeaderHeight(int height);

		void demo();

	private:
		void onTimelineViewTransformed(const QTransform& transform);

		TimelineView mView;
		TimelineScene mScene;

		TimelineOutline mOutline;

		QVBoxLayout mLayout;
		QVBoxLayout mTimelineLayout;
		QWidget mTimelineWidget;
		QSplitter mSplitter;
	};

}