#pragma once


#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QtWidgets/QGraphicsView>
#include "timelinemodel.h"
#include "trackitem.h"

namespace napkin
{

	class OutlineTrackItem : public QGraphicsProxyWidget
	{

	public:
		OutlineTrackItem(Track& track);

		Track& track() const { return mTrack; }

		void setWidth(int width);

	private:
		Track& mTrack;
		QWidget mWidget;
		QHBoxLayout mLayout;
	};

	class OutlineScene : public QGraphicsScene
	{
	public:
		OutlineScene();

		void setTimeline(Timeline* timeline);

		Timeline* timeline() const
		{ return mTimeline; }

		void resize(const QSize& size);

	private:
		void onTrackAdded(Track& track);

		void onTrackRemoved(Track& track);

		OutlineTrackItem* trackItem(Track& track);

		QList<OutlineTrackItem*> trackItems();

		Timeline* mTimeline = nullptr;

	};

	class OutlineView : public QGraphicsView
	{
	Q_OBJECT
	public:
		OutlineView();
	Q_SIGNALS:
		void resized(const QSize& size);
	protected:
		void resizeEvent(QResizeEvent* event) override;

	};

	class TimelineOutline : public QWidget
	{
		Q_OBJECT
	public:
		TimelineOutline();

		void setModel(Timeline* timeline);

	private:
		void onViewResized(const QSize& size);

		QVBoxLayout mLayout;
		OutlineView mView;
		OutlineScene mScene;
	};

} // napkin

