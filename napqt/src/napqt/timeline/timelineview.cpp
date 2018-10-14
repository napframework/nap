#include "timelineview.h"

#include <QKeyEvent>
#include <QtDebug>

#include <napqt/qtutils.h>

#include "timelinescene.h"

using namespace napqt;


TimelineView::TimelineView()
		: GridView()
{

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
	auto clickedTimelineItem = dynamic_cast<TimelineElementItem*>(item);
	auto clickedEvent = dynamic_cast<BaseEventItem*>(item);

	mDragMode = NoDrag;

	// Handle selection
	if (lmb && !alt)
	{
		if (clickedTimelineItem) // clicked an event
		{
			if (shift)
			{
				// append selection
				clickedTimelineItem->setSelected(true);
			} else if (ctrl)
			{
				// toggle selection
				clickedTimelineItem->setSelected(!clickedTimelineItem->isSelected());
			} else
			{
				// replace select
				if (!clickedTimelineItem->isSelected())
				{
					// deselect others
					for (auto m : selectedItems<TimelineElementItem>())
						m->setSelected(false);
					clickedTimelineItem->setSelected(true);
				}
			}

			bool leftGrip;
			auto resizeItem = resizeHandleAt(event->pos(), leftGrip);
			if (resizeItem)
			{
				mDragMode = leftGrip ? DragResizeLeft : DragResizeRight;
			} else
			{
				mDragMode = DragMove;
			}
		} else
		{
			// clicked on background
			if (!shift && !ctrl)
			{
				for (auto m : selectedItems<TimelineElementItem>())
					m->setSelected(false);
			}

			startRubberBand(event->pos());
		}
		mSelectedRanges.clear();
		for (auto m : selectedItems<EventItem>())
		{
			moveItemToFront(*m);
			mSelectedRanges.insert(m, m->range());
		}

		for (auto m : selectedItems<TickItem>())
		{
			moveItemToFront(*m);
			mSelectedTimes.insert(m, m->tick().time());
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

	auto frameInterval = 1.0 / timeline()->framerate();

	if (isRubberBandVisible()) {
		updateRubberBand(event->pos());
	}
	else if (lmb && !alt)
	{
		for (auto item : selectedItems<TimelineElementItem>())
		{
			auto eventItem = dynamic_cast<EventItem*>(item);
			if (eventItem)
			{
				Range range = mSelectedRanges[eventItem];
				if (mDragMode == DragMove)
				{
					range = range.offset(dragDelta.x());

					auto roundedStart = roundToInterval(range.start(), frameInterval);
					range.moveTo(roundedStart);

				} else if (mDragMode == DragResizeLeft)
				{
					qreal start = roundToInterval(range.start() + dragDelta.x(), frameInterval);
					if (eventItem->hasMinLength() && start > range.end() - eventItem->minLength())
						start = floorToInterval(range.end() - eventItem->minLength(), frameInterval);
					range.setStart(start);

				} else if (mDragMode == DragResizeRight)
				{
					qreal end = roundToInterval(range.end() + dragDelta.x(), frameInterval);
					if (eventItem->hasMinLength() && end < range.start() + eventItem->minLength())
						end = ceilToInterval(range.start() + eventItem->minLength(), frameInterval);
					range.setEnd(end);
				}
				eventItem->event().setRange(range);
			}
			auto tickItem = dynamic_cast<TickItem*>(item);
			if (tickItem)
			{
				qreal oldTime = mSelectedTimes[tickItem];
				qreal newTime = oldTime + dragDelta.x();

				auto roundedTime = roundToInterval(newTime, frameInterval);

				tickItem->tick().setTime(roundedTime);
			}
		}
	} else
	{
		bool left;
		auto eventItem = resizeHandleAt(event->pos(), left);
		if (eventItem)
			setOverrideCursor(left ? mResizeCursorShapeLeft : mResizeCursorShapeRight);
		else
			restoreCursor();
	}

}

void TimelineView::mouseReleaseEvent(QMouseEvent* event)
{
	if (isRubberBandVisible())
		selectItemsInRubberband();

	GridView::mouseReleaseEvent(event);
}

void TimelineView::selectItemsInRubberband()
{
	for (auto m : selectedItems<TimelineElementItem>())
		m->setSelected(false);

	QList<TimelineElementItem*> elements;
	for (auto item : items(rubberBandGeo())) {
		auto element = dynamic_cast<TimelineElementItem*>(item);
		if (element)
			element->setSelected(true);
	}
}

Timeline* TimelineView::timeline() const
{
	return dynamic_cast<TimelineScene*>(scene())->timeline();
}

void TimelineView::setOverrideCursor(const QCursor& c)
{
	if (mCursorOverridden)
		return;
	setCursor(c);
	mCursorOverridden = true;
}

void TimelineView::restoreCursor()
{
	setCursor(mDefaultCursor);
	mCursorOverridden = false;
}

BaseEventItem* TimelineView::resizeHandleAt(const QPointF& pos, bool& leftGrip) const
{
	auto eventItem = dynamic_cast<BaseEventItem*>(itemAt(pos.toPoint()));
	if (!eventItem || !eventItem->isResizable())
		return nullptr;

	auto sceneScale = getScale(transform());

	// position of mouse, relative to item, in scene space
	auto localPos = mapToScene(pos.toPoint()) - eventItem->scenePos();

	qreal gripWidth = mResizeGripWidth / sceneScale.width();
	if (localPos.x() <= gripWidth)
	{
		leftGrip = true;
		return eventItem;
	} else if (localPos.x() > eventItem->range().length() - gripWidth)
	{
		leftGrip = false;
		return eventItem;
	}

	return nullptr;
}



