#pragma once

#include <QGraphicsScene>
#include <QAction>
#include <napqt/gridview.h>
#include "standardcurve.h"


namespace napqt
{
	class CurveItem;
	class CurveSegmentItem;

	/**
	 * Any interactive (movable) handle in the scene
	 */
	class HandleItem : public QObject, public QGraphicsItem
	{
	Q_OBJECT
	public:
		HandleItem(CurveSegmentItem& parent);
		bool contains(const QPointF& point) const override;
		QRectF boundingRect() const override;
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
		CurveSegmentItem& curveSegmentItem();
		const CurveSegmentItem& curveSegmentItem() const;
		void setEmitItemChanged(bool b) { mEmitItemChanges = b; }
		virtual void updateRect();

	Q_SIGNALS:
		void moved(HandleItem* item);
		void selected(HandleItem* item);

	protected:
		QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
		QPen mPen;
		QPen mPenSelected;
		QBrush mBrush;
		QBrush mBrushSelected;
		QPainterPath mPath;
		qreal mExtent = 2;
		qreal mExtentSelectd = 2;
		qreal mHitExtent = 4;
		bool mEmitItemChanges = true;
		QRectF mRect;
		QRectF mHitRect;

	};

	/**
	 * Interactive handle for a curve point
	 */
	class PointHandleItem : public HandleItem
	{
	public:
		explicit PointHandleItem(CurveSegmentItem& parent);
		void setTangentsLocked(bool b) { mTangentsLocked = b; }
		bool isTangentsLocked() const { return mTangentsLocked; }
	private:
		bool mTangentsLocked = true;
	};

	/**
	 * Interactive handle for a curve tangent
	 */
	class TangentHandleItem : public HandleItem
	{
	public:
		explicit TangentHandleItem(CurveSegmentItem& parent);
		void setValue(const QPointF& value) { mValue = value; }
		const QPointF value() const { return mValue; }
		PointHandleItem& pointHandle();
		TangentHandleItem& oppositeTanHandle();
		bool isInTangent();
		void updateRect() override;
	private:
		QPointF mValue;
	};

	/**
	 * Non-interactive item to display a curve's tangent line
	 */
	class TangentLineItem : public QGraphicsPathItem
	{
	public:
		explicit TangentLineItem(QGraphicsItem& parent);
		void setHighlighted(bool b);
		void setFromTo(const QPointF& a, const QPointF& b);
		void showLimit(const QPointF& p);
		void hideLimit();

	private:
		bool mHighlighted = false;
		QGraphicsEllipseItem mLimitItem;
	};

	/**
	 * Non-interactive item to display a single curve segment
	 */
	class CurveSegmentItem : public QObject, public QGraphicsItem
	{
		Q_OBJECT
	public:
		explicit CurveSegmentItem(CurveItem& curveItem);
		CurveItem& curveItem() const;
		QRectF boundingRect() const override;

		PointHandleItem& pointHandle() { return mPointHandle; }
		TangentHandleItem& inTanHandle() { return mInTanHandle; }
		TangentHandleItem& outTanHandle() { return mOutTanHandle; }
		TangentLineItem& inTanLine() { return mInTanLine; }

		void setInTanVisible(bool b);
		void setOutTanVisible(bool b);
		void setTangentsVisible(bool b);

		QPainterPath shape() const override;
		void updateGeometry();
		int index();
		int orderedIndex();
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	Q_SIGNALS:
		void handleMoved(HandleItem* handle);

	private:
		void onHandleMoved(HandleItem* handle);
		void setPointsEmitItemChanges(bool b);
		void onTanHandleSelected(HandleItem* handle);
		bool isLastPoint();
		bool isFirstPoint();

		QPainterPath mPath;
		QPainterPath mDebugPath;
		bool mDrawDebug = true;
		bool mDrawQt = false;
		int mSampleCount = 16;
		QPen mPen;
		QPen mPenDebug;
		QPen mPenSelected;

		PointHandleItem mPointHandle;
		TangentHandleItem mInTanHandle;
		TangentHandleItem mOutTanHandle;
		TangentLineItem mInTanLine;
		TangentLineItem mOutTanLine;
	};

	/**
	 * QGraphicsItem representing one curve
	 */
	class CurveItem : public QObject, public QGraphicsPathItem
	{
	Q_OBJECT
	public:
		CurveItem(QGraphicsItem* parent, AbstractCurve& curve);
		AbstractCurve& curve() { return mCurve; }
		int segmentIndex(const CurveSegmentItem& seg) const;
		int sortedIndex(const CurveSegmentItem& seg);
		int sortedIndex(int unsortedIndex);
		QRectF boundingRect() const override;
		CurveSegmentItem* nextSegment(const CurveSegmentItem& seg);
		CurveSegmentItem* prevSegment(const CurveSegmentItem& seg);
		int prevSegIndex(int idx);
		int nextSegIndex(int idx);
		bool isLastPoint(int i);
		bool isFirstPoint(int i);
	private:
		void onPointsChanged(QList<int> indices);
		void onPointsAdded(QList<int> indices);
		void onPointsRemoved(QList<int> indices);
		void updateSegmentFromPoint(int i);
		void updateAllSegments();
		const QVector<int>& sortPoints();
		void setPointOrderDirty();
		AbstractCurve& mCurve;

		TangentLineItem mInfinityLineNeg;
		TangentLineItem mInfinityLinePos;
		QVector<CurveSegmentItem*> mSegments;
		bool mPointOrderDirty = true;
		QVector<int> mSortedToUnsorted;
		QVector<int> mUnsortedToSorted;
	};


	class CurveView : public GridView
	{
	Q_OBJECT
	public:
		enum InteractMode {
			None, Rubberband, RubberbandAdd, DragPoints, Pan, SelectAdd
		};


		explicit CurveView(QWidget* parent = nullptr);
		void setModel(AbstractCurveModel* model);
		AbstractCurveModel* model() { return mModel; }

	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void movePointHandles(const QList<PointHandleItem*>& items, const QPointF& sceneDelta);
		void moveTanHandles(const QList<TangentHandleItem*>& tangents, const QPointF& sceneDelta);

	private:
		CurveItem* closestCurveItem(const QPointF& pos);
		void initActions();
		void deleteSelectedItems();
		void onCurvesAdded(QList<int> indices);
		void onCurvesRemoved(QList<int> indices);
		void selectPointHandles(const QList<PointHandleItem*>& pointHandles);
		void onCustomContextMenuRequested(const QPoint& pos);
		void setSelectedPointInterps(AbstractCurve::InterpType interp);
		void setSelectedTangentsAligned(bool aligned);
		void alignTangents(AbstractCurve& curve, int pointIndex);
		// Get selected points, but also include points associated with tangent handles
		const QList<PointHandleItem*> pointsFromSelection();

		QGraphicsScene mCurveScene;
		QPoint mLastMousePos;
		AbstractCurveModel* mModel = nullptr;
		QList<CurveItem*> mCurveItems;

		InteractMode mInteractMode = None;

		QAction mDeleteAction;
		QAction mToggleAlignedAction;
		QAction mInterpLinearAction;
		QAction mInterpBezierAction;
		QAction mInterpSteppedAction;

	};

}
