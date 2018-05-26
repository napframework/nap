#include "timelineview.h"
#include "timelinescene.h"

#include <QtGui/QKeyEvent>
#include <QtDebug>
#include <generic/qtutils.h>

using namespace napkin;


TimelineView::TimelineView()
		: GridView()
{
	setMouseTracking(true);
}

void TimelineView::setTimeScale(qreal scale)
{
	zoom(QPointF(scale, 1), QPointF());
}

const Range TimelineView::getViewRange() const
{
	return {mapToScene(0, 0).x(),
			mapToScene(viewport()->rect().width(), 0).x()};
}


void TimelineView::mousePressEvent(QMouseEvent* event)
{
	if (!scene())
		return;

	mMousePressPos = event->pos();

	bool ctrl = event->modifiers() == Qt::ControlModifier;
	bool shift = event->modifiers() == Qt::ShiftModifier;
	bool alt = event->modifiers() == Qt::AltModifier;
	bool lmb = event->buttons() == Qt::LeftButton;

	auto item = itemAt(event->pos());
	auto clickedEvent = dynamic_cast<BaseEventItem*>(item);

	// Handle selection
	if (lmb && !alt)
	{
		if (clickedEvent)
		{
			if (shift)
			{
				clickedEvent->setSelected(true);
			} else if (ctrl)
			{
				clickedEvent->setSelected(!clickedEvent->isSelected());
			} else
			{
				if (!clickedEvent->isSelected())
				{
					for (auto m : selectedEventItems())
						m->setSelected(false);
					clickedEvent->setSelected(true);
				}
			}
		} else
		{
			if (!shift && !ctrl)
			{
				for (auto m : selectedEventItems())
					m->setSelected(false);
			}
		}
		mSelectedPositions.clear();
		for (auto m : selectedEventItems())
		{
			moveItemToFront(*m);
			mSelectedPositions.insert(m, m->scenePos());
		}

		return;
	}
	GridView::mousePressEvent(event);
}

void TimelineView::mouseMoveEvent(QMouseEvent* event)
{
	GridView::mouseMoveEvent(event);

	bool ctrl = event->modifiers() == Qt::ControlModifier;
	bool shift = event->modifiers() == Qt::ShiftModifier;
	bool alt = event->modifiers() == Qt::AltModifier;
	bool lmb = event->buttons() == Qt::LeftButton;
	auto dragDelta = mapToScene(event->pos()) - mapToScene(mousePressedPos());

	auto framestep = 1.0 / timeline()->framerate();

	if (lmb && !alt)
	{
		for (auto item : selectedEventItems())
		{
			auto oldPos = mSelectedPositions[item];
			auto newPos = oldPos + dragDelta;

			auto h = item->event().track().height();

			int track = qRound(newPos.y() / h);
			auto roundedx = qRound(newPos.x() / framestep) * framestep;

			newPos.setY(item->pos().y());
//			newPos.setY(track * h);
			newPos.setX(roundedx);

			item->setPos(newPos);
			moveItemToFront(*item);
			item->event().moveTo(roundedx);

		}
	}


}

void TimelineView::mouseReleaseEvent(QMouseEvent* event)
{
	GridView::mouseReleaseEvent(event);
}

const QList<EventItem*> TimelineView::selectedEventItems() const
{
	QList<EventItem*> items;
	for (auto m : scene()->selectedItems())
	{
		auto eventItem = dynamic_cast<EventItem*>(m);
		if (eventItem)
			items << eventItem;
	}
	return items;
}

Timeline* TimelineView::timeline() const
{
	return dynamic_cast<TimelineScene*>(scene())->timeline();
}



