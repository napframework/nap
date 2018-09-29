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

		void setTimeScale(qreal scale);

		const Range getViewRange() const;

	protected:
		void mousePressEvent(QMouseEvent* event) override;

		void mouseMoveEvent(QMouseEvent* event) override;

		void mouseReleaseEvent(QMouseEvent* event) override;


	private:
		Timeline* timeline() const;
		const QList<BaseEventItem*> selectedEventItems() const;

		template<typename T>
		QList<T*> selectedItems() const {
			QList<T*> items;
			for (auto m : scene()->selectedItems())
			{
				auto eventItem = dynamic_cast<T*>(m);
				if (eventItem)
					items << eventItem;
			}
			return items;
		}

		QMap<BaseEventItem*, QPointF> mSelectedPositions;

	};

}