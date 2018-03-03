#pragma once

#include <QGraphicsView>

#include "timelinemodel.h"
#include "eventitem.h"
#include "trackitem.h"
#include "gridview.h"


namespace napkin {

	class TimelineView : public GridView {
	Q_OBJECT

	public:
		TimelineView();
		~TimelineView() {}

		void setTopMargin(int margin);
	protected:
		void mousePressEvent(QMouseEvent* event) override;

		void mouseMoveEvent(QMouseEvent* event) override;

		void mouseReleaseEvent(QMouseEvent* event) override;


	private:
		Timeline* timeline() const;
		const QList<EventItem*> selectedEventItems() const;
		QMap<EventItem*, QPointF> mSelectedPositions;

	};

}