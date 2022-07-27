/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QGraphicsScene>
#include <QAction>
#include <QVBoxLayout>
#include <QToolBar>

#include <napqt/gridview.h>
#include <QtWidgets/QDoubleSpinBox>
#include <napqt/floatlineedit.h>
#include <napqt/flowlayout.h>
#include "standardcurve.h"


namespace nap
{
	namespace qt
	{
		class CurveItem;

		class CurveSegmentItem;

		/**
		 * Any interactive (movable) handle in the scene.
		 * For internal use. Don't use this directly, but let CurveView manage this.
		 */
		class HandleItem : public QObject, public QGraphicsPathItem
		{
		Q_OBJECT
		public:
			/**
			 * @param parent The segment to which this handle will be attached
			 */
			explicit HandleItem(CurveSegmentItem& parent);
			void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
			CurveSegmentItem& curveSegmentItem();
			const CurveSegmentItem& curveSegmentItem() const;
			void setEmitItemChanged(bool b) { mEmitItemChanges = b; }
			virtual void updateRect();
			QPainterPath shape() const override;

		Q_SIGNALS:
			/**
			 * Emitted when this handle has moved
			 * @param item The handle that has moved
			 */
			void moved(HandleItem* item);

			/**
			 * Emitted when this handle's selection state has changed
			 * @param item
			 */
			void selected(HandleItem* item);

		protected:
			QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
			QPen mPen;
			QPen mPenSelected;
			QBrush mBrush;
			QBrush mBrushSelected;
			qreal mExtent = 2;
			qreal mExtentSelectd = 2;
			qreal mShapeExtent = 8; // used for hit detection (amongst others)
			QPainterPath mShape; // shape used for hit detection (amongst others)
			bool mEmitItemChanges = true;
			QRectF mRect;
		};

		/**
		 * Interactive handle for a curve point
		 * For internal use. Don't use this directly, but let CurveView manage this.
		 */
		class PointHandleItem : public HandleItem
		{
			Q_OBJECT
		public:
			explicit PointHandleItem(CurveSegmentItem& parent);

		};

		/**
		 * Interactive handle for a curve tangent
		 * For internal use. Don't use this directly, but let CurveView manage this.
		 */
		class TangentHandleItem : public HandleItem
		{
			Q_OBJECT
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
		 * For internal use. Don't use this directly, but let CurveView manage this.
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
		 * For internal use. Don't use this directly, but let CurveView manage this.
		 */
		class CurveSegmentItem : public QObject, public QGraphicsPathItem
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
			int index() const;
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
		 * Don't use this directly, but let CurveView manage this.
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
			const QVector<CurveSegmentItem*>& segments() { return mSegments; }
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

		/**
		 * The main view for curve editing. It's just a bare curve editor, when a toolbar is needed, use CurveEditor
		 * It uses an AbstractCurveModel to display and edit the curves.
		 */
		class CurveView : public GridView
		{
		Q_OBJECT
		public:
			explicit CurveView(QWidget* parent = nullptr);

			/**
			 * Start editing the given AbstractCurveModel in the editor.
			 *
			 * @param model The curve that should be edited, provide nullptr to empty the view.
			 */
			void setModel(AbstractCurveModel* model);

			/**
			 * @return The curve [model] that is currently being edited by this curve view.
			 */
			AbstractCurveModel* model() { return mModel; }

			/**
			 * Set the current selection to the provided curves.
			 * @param curve The curves to select. These must be provided in the model set using setModel()
			 */
			void selectCurves(const QList<AbstractCurve*>& curve);

			/**
			 * @return The currently selected curve point indexes in the editor and their associated curves.
			 */
			QMap<AbstractCurve*, QList<int>> selectedPoints();

			/**
			 * Set the time of all selected points to t.
			 * @param t The time to set on the selected points
			 */
			void setSelectedPointTimes(qreal t);

			/**
			 * Set the value of all selected points to v.
			 * @param v The value to set on the selected curve points.
			 */
			void setSelectedPointValues(qreal v);

			/**
			 * @return All interpolation changing actions available for this curve view.
			 */
			QList<QAction*> interpActions();

			/**
			 * @return All tangent changing actions available for this curve view.
			 */
			QList<QAction*> tangentActions();

			/**
			 * @return All remaining uncategorized actions
			 */
			QList<QAction*> auxiliaryActions();

			// All actions
			QAction mDeleteAction;
			QAction mSetTangentsAlignedAction;
			QAction mSetTangentsBrokenAction;
			QAction mInterpLinearAction;
			QAction mInterpBezierAction;
			QAction mInterpSteppedAction;
			QAction mFlattenTangentsAction;
			QAction mFrameViewAction;

		Q_SIGNALS:
			void selectionChanged(QMap<AbstractCurve*, QList<int>> points);

		protected:
			void drawBackground(QPainter* painter, const QRectF& rect) override;
			void drawCurveExtrapolation(QPainter* painter, const QRectF& dirtyRect, const QRectF& viewRect,
										const AbstractCurve& curve,
										qreal step);

			void mousePressEvent(QMouseEvent* event) override;
			void mouseMoveEvent(QMouseEvent* event) override;
			void mouseReleaseEvent(QMouseEvent* event) override;
			void movePointHandles(const QList<PointHandleItem*>& items, const QPointF& sceneDelta);
			void moveTanHandles(const QList<TangentHandleItem*>& tangents, const QPointF& sceneDelta);

		private:
			enum InteractMode
			{
				None, Rubberband, RubberbandAdd, DragPoints, Pan, SelectAdd
			};

			AbstractCurve* closestCurve(const QPointF& pos);
			static QPointF closestPointOnCurve(const AbstractCurve& curve, const QPointF& pt);
			static int firstPoint(const AbstractCurve& curve);
			static int lastPoint(const AbstractCurve& curve);
			void initActions();
			void deleteSelectedItems();
			void onCurvesAdded(QList<int> indices);
			void onCurvesRemoved(QList<int> indices);
			void selectPointHandles(const QList<PointHandleItem*>& pointHandles);
			void onCustomContextMenuRequested(const QPoint& pos);
			void setSelectedPointInterps(AbstractCurve::InterpType interp);
			void setSelectedTangentsAligned(bool aligned);
			void setSelectedTangentsFlat();
			void alignTangents(AbstractCurve& curve, int pointIndex, bool finished);
			// Get selected points, but also include points associated with tangent handles
			const QList<PointHandleItem*> pointsFromSelection();
			const QRectF frameItemsBoundsSelected() const override;
			const QRectF frameItemsBounds() const override;
			const QRectF handleItemBounds(const QList<QGraphicsItem*>& items) const;
			const QList<QGraphicsItem*> frameableItems(const QList<QGraphicsItem*>& items) const;
			CurveItem* curveItem(const AbstractCurve& curve);

			void commitPointEditChanges(bool finished);

			QGraphicsScene mCurveScene;
			QPoint mLastMousePos;
			AbstractCurveModel* mModel = nullptr;
			QList<CurveItem*> mCurveItems;

			InteractMode mInteractMode = None;

			// Currently editing tangent list
			QMap<CurveItem*, QList<QMap<int, QPointF>>> mTangentEditMap;

			// Currently editing point list
			QMap<CurveItem*, QMap<int, QPointF>> mPointEditMap;
		};

		/**
		 * A curve editor widget with a toolbar that allows for editing curves
		 */
		class CurveEditor : public QWidget
		{
		Q_OBJECT
		public:
			/**
			 * @param parent See QWidget constructor
			 */
			CurveEditor(QWidget* parent = nullptr);
			~CurveEditor();

			/**
			 * Start editing the curve.
			 * @param model The curve model that holds the curve. Changes will be applied to this model.
			 * 			Provide nullptr to clear.
			 */
			void setModel(AbstractCurveModel* model);

			/**
			 * @return the curve view
			 */
			CurveView& getView()	{ return mCurveView; }

		private:
			void onTimeChanged(qreal t);
			void onValueChanged(qreal v);

			void onSelectionChanged(QMap<AbstractCurve*, QList<int>> points);
			void onPointsChanged();

			CurveView mCurveView;
			QVBoxLayout mLayout;
			QWidget mToolbar;
			FlowLayout mToolbarLayout;
			FloatLineEdit mTimeSpinbox;
			FloatLineEdit mValueSpinbox;
			AbstractCurveModel* mCurveModel = nullptr;
		};

	} // namespace qt
} // namespace nap
