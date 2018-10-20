#pragma once

#include <QGraphicsView>

#include <napqt/gridview.h>

#include "timelinemodel.h"
#include "eventitem.h"
#include "trackitem.h"


namespace napqt
{

	class TimelineView : public GridView
	{
	Q_OBJECT
	public:
		TimelineView();
		~TimelineView() {}

		void setTimeScale(qreal scale);

		const Range getViewRange() const;

		enum DragMode {
			NoDrag, DragMove, DragResizeLeft, DragResizeRight
		};

	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;


	private:
		Timeline* timeline() const;
		void setOverrideCursor(const QCursor& cursor);
		void restoreCursor();
		BaseEventItem* resizeHandleAt(const QPointF& pos, bool& leftGrip) const;

		QMap<BaseEventItem*, Range> mSelectedRanges;
		QMap<TickItem*, qreal> mSelectedTimes;
		qreal mResizeGripWidth = 10;
		QCursor mResizeCursorShapeRight = QCursor(Qt::SizeHorCursor);
		QCursor mResizeCursorShapeLeft = QCursor(Qt::SizeHorCursor);
		QCursor mDefaultCursor;
		bool mCursorOverridden = false;
		DragMode mDragMode = NoDrag;

		bool mCtrlHeld;
		bool mShiftHeld;
		bool mAltHeld;

	};

}