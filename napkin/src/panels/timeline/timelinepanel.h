#pragma once

#include <QWidget>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QPainter>
#include <QPaintEvent>
#include <QtWidgets/QStylePainter>
#include <QtWidgets/QStyleOptionFrame>
#include <QtGui/QtGui>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <cassert>
#include "timelinemodel.h"

struct ViewTransform {
	QPointF pos;
	QPointF scale;

	ViewTransform() : pos(0, 0), scale(1, 1) {}

	void translate(QPointF delta);
	void translate(qreal x, qreal y);
	void zoom(QPointF delta);
};

class RulerWidget : public QWidget {
public:
	RulerWidget();

	void setViewTransform(const ViewTransform& xf);
protected:
	void paintEvent(QPaintEvent* event) override;

private:
	ViewTransform mViewTransform;
};


class OutlineHeader : public QWidget {
public:
	OutlineHeader() : QWidget() {}

protected:
	void paintEvent(QPaintEvent* event) override;

};


class TLTrackWidget : public QWidget {
public:
	TLTrackWidget(QWidget* parent, TLTrack& track) : mTrack(track), QWidget(parent) {}

	TLTrack& track() const { return mTrack; }

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	TLTrack& mTrack;

};

class TLEventWidget : public QWidget {
public:
	TLEventWidget(QWidget* parent, TLEvent& event) : mEvent(event), QWidget(parent) {}

	TLEvent& event() const { return mEvent; }

protected:
	void mousePressEvent(QMouseEvent* event) override;

	void paintEvent(QPaintEvent* event) override;

private:
	TLEvent& mEvent;
};


class TLOutlineItem : public QWidget {
public:
	TLOutlineItem(TLTrack& track, QWidget* parent);

	TLTrack& track() const { return mTrack; }
	void setHeight(int height);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	QHBoxLayout mLayout;
	QLabel mLabel;
	TLTrack& mTrack;
};

class TimelineOutline : public QWidget {
public:
	TimelineOutline();

	void setModel(TLTimeline* timeline);
	void setHeaderHeight(int height);

	void setViewTransform(const ViewTransform& xf);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	void onTrackAdded(TLTrack& track);
	void onTrackRemoved(TLTrack& track);
	TLOutlineItem* widget(TLTrack& track);

	QWidget mHolder;
	QVBoxLayout mLayout;
	OutlineHeader mHeader;
	QList<TLOutlineItem*> mTracks;
	TLTimeline* mTimeline = nullptr;
};

class TimelineCanvas : public QWidget {
Q_OBJECT
public:
	TimelineCanvas();

	void setModel(TLTimeline* timeline);

	void pan(const QPointF& delta);
	void zoom(const QPointF& delta);

Q_SIGNALS:
	void viewTransformed(const ViewTransform& xf);

protected:
	void resizeEvent(QResizeEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	void clear();
	void onTrackAdded(TLTrack& track);
	void onTrackRemoved(TLTrack& track);
	void onEventAdded(TLEvent& event);
	void onEventRemoved(TLEvent& event);
	void invalidateLayout();
	void layout();

	const QList<TLTrackWidget*> trackWidgets();
	const QList<TLEventWidget*> eventWidgets();
	TLTrackWidget* trackWidget(TLTrack& track);
	TLEventWidget* eventWidget(TLEvent& event);

	QWidget mTrackHolder;
	QWidget mEventHolder;
	TLTimeline* mTimeline = nullptr;
	bool mLayoutValid = false;
	ViewTransform mViewTransform;

	QPointF mMousePressPos;
	QPointF mMouseLastPos;
};


class TimelineView : public QWidget {
	Q_OBJECT
public:
	TimelineView();

	void setModel(TLTimeline* timeline);
	void setHeaderHeight(int height);
	int headerHeight() const { return mRuler.minimumHeight(); }

Q_SIGNALS:
	void timelineTransformChanged(const ViewTransform& xf);

private:
	void onTimelineTransformed(const ViewTransform& xf);

	QVBoxLayout mLayout;
	QScrollArea mScrollArea;
	TimelineCanvas mCanvas;
	RulerWidget mRuler;
	TLTimeline* mTimeline = nullptr;

};

class TimelinePanel : public QWidget {
public:
	TimelinePanel();

	void setTimeline(TLTimeline* timeline);
	void demo();
	void setHeaderHeight(int height);

private:
	void onTimelineTransformChanged(const ViewTransform& xf);

	QVBoxLayout mLayout;
	QSplitter mSplitter;
	TimelineView mTimeline;
	TimelineOutline mOutline;
};