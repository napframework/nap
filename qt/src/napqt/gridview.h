/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "timedisplay.h"
#include "autosettings.h"
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QRubberBand>
#include <memory>

namespace nap
{
	namespace qt
	{

		/**
		 * A QGraphicsView displaying grid lines and providing basic view navigation hotkeys and mouse interaction.
		 */
		class GridView : public QGraphicsView
		{
		Q_OBJECT

		public:
			enum class ZoomMode
			{
				IgnoreAspectRatio, KeepAspectRatio, Horizontal, Vertical
			};

			enum class PanMode
			{
				Horizontal, Vertical, Parallax
			};

			explicit GridView(QWidget* parent = nullptr);
			~GridView() = default;

			void setPanZoomMode(PanMode panMode, ZoomMode zoomMode)
			{
				mPanMode = panMode;
				mZoomMode = zoomMode;
			}
			void setFramePanZoomMode(PanMode panMode, ZoomMode zoomMode)
			{
				mFramePanMode = panMode;
				mFrameZoomMode = zoomMode;
			}

			void pan(const QPointF& delta);
			void zoom(const QPointF& delta, const QPointF& pivot);
			void centerView();
			void frameAll();
			void frameSelected();
			void frameView(const QRectF& rect);
			void setVerticalScroll(int value);
			void setPanBounds(qreal left, qreal top, qreal right, qreal bottom);
			void setPanBounds(const QRectF& rec);
			void constrainTransform(QTransform& xf);
			void constrainView();
			void setVerticalFlipped(bool flipped);
			const bool isVerticalFlipped() const { return mVerticalFlipped; }

			void setGridIntervalDisplay(std::shared_ptr<IntervalDisplay> horiz, std::shared_ptr<IntervalDisplay> vert);

			const QPoint& mousePressedPos() const { return mMousePressPos; }
			const QPoint& mouseLastPos() const { return mMouseLastPos; }
			const QPoint& mouseDelta() const { return mMouseDelta; }

			const QMargins frameMargins() const { return mFrameMargins; }
			void setFrameMargins(const QMargins& margins) { mFrameMargins = margins; }

		Q_SIGNALS:
			void viewTransformed();

		protected:
			void startRubberBand(const QPoint& pos);
			void updateRubberBand(const QPoint& pos);
			void hideRubberBand();
			bool isRubberBandVisible() const { return mRubberBand.isVisible(); }
			const QRect rubberBandGeo() const;

			void drawBackground(QPainter* painter, const QRectF& rect) override;

			void mousePressEvent(QMouseEvent* event) override;
			void mouseMoveEvent(QMouseEvent* event) override;
			void mouseReleaseEvent(QMouseEvent* event) override;
			void wheelEvent(QWheelEvent* event) override;
			void keyPressEvent(QKeyEvent* event) override;
			void keyReleaseEvent(QKeyEvent* event) override;
			QPoint mMousePressPos;

			virtual void setSelection(const QList<QGraphicsItem*>& items);
			void addSelection(const QList<QGraphicsItem*>& items);
			void removeSelection(const QList<QGraphicsItem*>& items);
			void toggleSelection(const QList<QGraphicsItem*>& items);
			void clearSelection();

			template<typename T>
			const QList<T*> selectedItems() const;

			template<typename O>
			const QList<QGraphicsItem*> filter(const QList<QGraphicsItem*>& items) const;
			template<typename O>
			const QList<O*> filterT(const QList<QGraphicsItem*>& items) const;

		private:
			void drawHatchesHorizontal(QPainter* painter, const QRectF& rect, qreal minStepSize, const QColor& color,
									   bool labels);
			void drawHatchesVertical(QPainter* painter, const QRectF& rect, qreal minStepSize, const QColor& color,
									 bool labels);
			virtual const QRectF frameItemsBoundsSelected() const;
			virtual const QRectF frameItemsBounds() const;
			const QPointF viewScale() const;
			const QPointF viewPos() const;
			void applyTransform(const QTransform& xf);

			bool mVerticalFlipped = false;

			QPoint mMouseLastPos;
			QPoint mMouseDelta;
			QSize mViewSize;
			QFont mRulerFont;
			QRubberBand mRubberBand;
			bool mGridEnabled = true;
			bool mDrawLabelsH = true;
			bool mDrawLabelsV = true;
			bool mDrawHLines = true;
			bool mDrawVLines = true;
			int mGridMinStepSizeHMinor = 30;
			int mGridMinStepSizeVMinor = 30;
			int mGridMinStepSizeHMajor = 120;
			int mGridMinStepSizeVMajor = 120;
			bool mDrawHoldout = true;
			QRectF mHoldoutRect = QRectF(0, 0, 1, 1);
			QMargins mFrameMargins = {50, 50, 50, 50};

			ZoomMode mZoomMode = ZoomMode::Horizontal;
			PanMode mPanMode = PanMode::Parallax;
			ZoomMode mFrameZoomMode = ZoomMode::Horizontal;
			PanMode mFramePanMode = PanMode::Parallax;

			QRectF mPanBounds;
			std::shared_ptr<IntervalDisplay> mIvalDisplayHorizontal = nullptr;
			std::shared_ptr<IntervalDisplay> mIvalDisplayVertical = nullptr;

			qreal mFramerate = 30;

			qreal calcGridStep(qreal desiredSpacing, qreal viewWidth, qreal sceneRectWidth) const;
			qreal calcGridStepTime(qreal desiredSpacing, qreal viewWidth, qreal sceneRectWidth, qreal minStepSize) const;
		};

		class GridViewStorer : public WidgetStorer<GridView>
		{
		public:
			void store(const GridView& widget, const QString& key, QSettings& s) const override
			{
				s.setValue(key + "_MATRIX", widget.transform());
			}
			void restore(GridView& widget, const QString& key, const QSettings& s) const override
			{
				if (s.contains(key + "_MATRIX"))
					widget.setTransform(s.value(key + "_MATRIX").value<QTransform>());
			}
		};

	}


	template<typename T>
	const QList<T*> nap::qt::GridView::selectedItems() const
	{
		QList<T*> items;
		for (auto m : scene()->selectedItems())
		{
			auto eventItem = dynamic_cast<T*>(m);
			if (eventItem)
				items << eventItem;
		}
		return items;
	}

	template<typename T>
	const QList<QGraphicsItem*> nap::qt::GridView::filter(const QList<QGraphicsItem*>& items) const
	{
		QList<QGraphicsItem*> out;
		for (auto item : items)
		{
			auto cast = dynamic_cast<T*>(item);
			if (cast)
				out << item;
		}
		return out;
	}

	template<typename T>
	const QList<T*> nap::qt::GridView::filterT(const QList<QGraphicsItem*>& items) const
	{
		QList<T*> out;
		for (auto item : items)
		{
			auto cast = dynamic_cast<T*>(item);
			if (cast)
				out << cast;
		}
		return out;

	} // namespace qt

} // namespace nap
