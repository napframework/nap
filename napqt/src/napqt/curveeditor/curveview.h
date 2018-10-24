#pragma once

#include <QGraphicsScene>
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
		void setEmitItemChanged(bool b) { mEmitItemChanges = b; }

	Q_SIGNALS:
		void moved(HandleItem* item);
		void selected(HandleItem* item);

	protected:
		QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
		QPen mPen;
		QPen mPenSelected;
		QBrush mBrush;
		QBrush mBrushSelected;

	private:
		void updateRect();

		QPainterPath mPath;
		QRectF mRect;
		QRectF mHitRect;

		qreal mExtent = 2;
		qreal mExtentSelectd = 2;
		qreal mHitExtent = 4;
		bool mEmitItemChanges = true;
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
		bool isInTangent();
	private:
		QPointF mValue;
	};

	/**
	 * Non-interactive item to display a curve's tangent line
	 */
	class LineItem : public QGraphicsPathItem
	{
	public:
		explicit LineItem(QGraphicsItem& parent);
		void setHighlighted(bool b);
		void setFromTo(const QPointF& a, const QPointF& b);
	private:
		QPen mPen;
		QPen mPenSelected;
		bool mHighlighted = false;
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
		LineItem& inTanLine() { return mInTanLine; }
		LineItem& outTanLine() { return mOutTanLine; }

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
		void onHandleSelected(HandleItem* handle);
		void setPointsEmitItemChanges(bool b);
		void onTanHandleSelected(HandleItem* handle);
		void updateHandleVisibility();
		bool isInTanVisible();
		bool isOutTanVisible();

		QPainterPath mPath;
		QPainterPath mDebugPath;
		bool mDrawDebug = false;
		int mSampleCount = 32;
		QPen mPen;
		QPen mPenDebug;
		QPen mPenSelected;

		PointHandleItem mPointHandle;
		TangentHandleItem mInTanHandle;
		TangentHandleItem mOutTanHandle;
		LineItem mInTanLine;
		LineItem mOutTanLine;

		TangentHandleItem* mTangentHandles[2];
		QGraphicsItem* mTangentItems[4];

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
		int unsortedIndex(int sortedIndex);
		QRectF boundingRect() const override;
		CurveSegmentItem* nextSegment(const CurveSegmentItem& seg);
		CurveSegmentItem* prevSegment(const CurveSegmentItem& seg);
		int prevSegIndex(int idx);
		int nextSegIndex(int idx);
	private:
		void onPointsChanged(QList<int> indices);
		void onPointsAdded(QList<int> indices);
		void onPointsRemoved(QList<int> indices);
		void updateSegmentFromPoint(int i);
		void updateAllSegments();
		bool isFirstPoint(int i);
		bool isLastPoint(int i);
		const QVector<int>& sortPoints();
		void setPointOrderDirty();
		AbstractCurve& mCurve;

		LineItem mInfinityLineNeg;
		LineItem mInfinityLinePos;
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
			None, Rubberband, RubberbandAdd, DragPoints, Pan, Zoom
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
		void onCurvesAdded(QList<int> indices);
		void onCurvesRemoved(QList<int> indices);

		const QList<TangentHandleItem*> tanHandles();
		const QList<PointHandleItem*> pointHandles();


		QGraphicsScene mCurveScene;
		QPoint mLastMousePos;
		AbstractCurveModel* mModel = nullptr;
		QList<CurveItem*> mCurveItems;

		InteractMode mInteractMode = None;
		bool mCtrlHeld;
		bool mShiftHeld;
		bool mAltHeld;

	};

}
