#pragma once

#include <QGraphicsScene>
#include <napqt/gridview.h>
#include "standardcurve.h"


namespace napqt
{

	/**
	 * Any interactive (movable) handle in the scene
	 */
	class HandleItem : public QObject, public QGraphicsItem
	{
	Q_OBJECT
	public:
		HandleItem();
		bool contains(const QPointF& point) const override;
		QRectF boundingRect() const override;
		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	protected:
		QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

	Q_SIGNALS:
		void moved(HandleItem* item);
		void selected(HandleItem* item);

	protected:
		QPen mPen;
		QBrush mBrush;
		QBrush mBrushSelected;

	private:
		QPainterPath mPath;
		QRectF mRect;
		QRectF mHitRect;

		qreal mExtent = 4;
		qreal mHitExtent = 8;
	};

	/**
	 * Interactive handle for a curve point
	 */
	class PointHandleItem : public HandleItem
	{
	public:
		PointHandleItem();
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
		TangentHandleItem();
		void setValue(const QPointF& value) { mValue = value; }
		const QPointF value() const { return mValue; }
	private:
		QPointF mValue;
	};

	/**
	 * Non-interactive item to display a curve's tangent line
	 */
	class LineItem : public QGraphicsPathItem
	{
	public:
		LineItem();
		void setColor(const QColor& color);
		void setFromTo(const QPointF& a, const QPointF& b);
	};

	/**
	 * Non-interactive item to display a single curve segment
	 */
	class CurveSegmentItem : public QObject, public QGraphicsItem
	{
		Q_OBJECT
	public:
		CurveSegmentItem();
		void setPoints(const QPointF (& pts)[4]);
		QRectF boundingRect() const override;

		PointHandleItem& pointHandle() { return mPointHandle; }
		TangentHandleItem& inTanHandle() { return mInTanHandle; }
		TangentHandleItem& outTanHandle() { return mOutTanHandle; }
		LineItem& inTanLine() { return mInTanLine; }
		LineItem& outTanLine() { return mOutTanLine; }

		void setFirstPoint(bool b);
		void setLastPoint(bool b);
		QPainterPath shape() const override;

		void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	Q_SIGNALS:
		void handleMoved(HandleItem* handle);

	private:
		void refreshCurve();

		QPointF mPoints[4];
		QPainterPath mPath;
		QPainterPath mDebugPath;
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

	private:
		void onPointsChanged(QList<int> indices);
		void onPointsAdded(QList<int> indices);
		void onPointsRemoved(QList<int> indices);
		int segmentIndex(CurveSegmentItem* seg) const;
		int sortedIndex(CurveSegmentItem* seg);
		void updateSegmentFromPoint(int i);
		CurveSegmentItem* nextSegment(int i);
		const QList<int>& sortedPoints();
		void setPointOrderDirty();
		AbstractCurve& mCurve;

		LineItem mInfinityLineNeg;
		LineItem mInfinityLinePos;
		QList<CurveSegmentItem*> mSegments;
		bool mPointOrderDirty = true;
		QList<int> mSortedPoints;
	};

	/**
	 * QGraphicsScene managing all the curves
	 */
	class CurveScene : public QGraphicsScene
	{
	public:
		CurveScene();
		void setModel(AbstractCurveModel* model);
		AbstractCurveModel* model() { return mModel; }
	private:
		void onCurvesAdded(QList<int> indices);
		void onCurvesRemoved(QList<int> indices);

		AbstractCurveModel* mModel = nullptr;
		QList<CurveItem*> mCurveItems;
	};

	class CurveView : public GridView
	{
	Q_OBJECT
	public:
		explicit CurveView(QWidget* parent = nullptr);
		void setModel(AbstractCurveModel* model) { mCurveScene.setModel(model); }
		AbstractCurveModel* model() { return mCurveScene.model(); }

	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
	private:
		CurveScene mCurveScene;
	};

}
