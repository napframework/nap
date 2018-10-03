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
	auto clickedEventItem = dynamic_cast<BaseEventItem*>(item);

	mDragMode = NoDrag;

	// Handle selection
	if (lmb && !alt)
	{
		if (clickedEventItem) // clicked an event
		{
			if (shift)
			{
				// append selection
				clickedEventItem->setSelected(true);
			} else if (ctrl)
			{
				// toggle selection
				clickedEventItem->setSelected(!clickedEventItem->isSelected());
			} else
			{
				// replace select
				if (!clickedEventItem->isSelected())
				{
					// deselect others
					for (auto m : selectedEventItems())
						m->setSelected(false);
					clickedEventItem->setSelected(true);
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
			if (!shift && !ctrl)
			{
				for (auto m : selectedEventItems())
					m->setSelected(false);
			}
		}
		mSelectedRanges.clear();
		for (auto m : selectedEventItems())
		{
			moveItemToFront(*m);
			mSelectedRanges.insert(m, m->range());
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

	if (lmb && !alt)
	{
		for (auto item : selectedItems<EventItem>())
		{
			if (mDragMode == DragMove)
			{
				Range oldRange = mSelectedRanges[item];
				Range newRange = oldRange + dragDelta.x();

				auto roundedStart = roundToInterval(newRange.start(), frameInterval);
				newRange.moveTo(roundedStart);

				item->event().setRange(newRange);
				moveItemToFront(*item);
			}
			else if (mDragMode == DragResizeLeft)
			{
				Range range = mSelectedRanges[item];
				qreal start = roundToInterval(range.start() + dragDelta.x(), frameInterval);
				if (item->hasMinLength() && start > range.end() - item->minLength())
					start = floorToInterval(range.end() - item->minLength(), frameInterval);
				range.setStart(start);

				item->event().setRange(range);
				moveItemToFront(*item);

			}
			else if (mDragMode == DragResizeRight)
			{
				Range range = mSelectedRanges[item];
				qreal end = roundToInterval(range.end() + dragDelta.x(), frameInterval);
				if (item->hasMinLength() && end < range.start() + item->minLength())
					end = ceilToInterval(range.start() + item->minLength(), frameInterval);
				range.setEnd(end);

				item->event().setRange(range);
				moveItemToFront(*item);
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
	GridView::mouseReleaseEvent(event);
}

const QList<BaseEventItem*> TimelineView::selectedEventItems() const
{
	QList<BaseEventItem*> items;
	for (auto m : scene()->selectedItems())
	{
		auto eventItem = dynamic_cast<BaseEventItem*>(m);
		if (eventItem)
			items << eventItem;
	}
	return items;
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



