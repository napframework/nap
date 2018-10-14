#pragma once

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QRubberBand>
#include <asio/detail/shared_ptr.hpp>
#include <memory>
#include "timedisplay.h"

namespace napqt
{


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

		enum class RulerFormat
		{
			Float, SMPTE
		};

		explicit GridView(QWidget* parent = nullptr);
		~GridView() {}

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
		void setDrawHLabels(bool b) { mDrawLabelsH = b; }
		void setDrawVLabels(bool b) { mDrawLabelsV = b; }
		void setDrawHLines(bool b) { mDrawHLines = b; }
		void setDrawVLines(bool b) { mDrawVLines = b; }

		void pan(const QPointF& delta);
		void zoom(const QPointF& delta, const QPointF& pivot);
		void centerView();
		void frameAll(QMargins margins);
		void frameSelected(QMargins margins);
		void frameView(const QRectF& rect, QMargins margins);
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
	private:
		void drawHatchesHorizontal(QPainter* painter, const QRectF& rect, qreal minStepSize, const QColor& color,
								   bool labels);
		void drawHatchesVertical(QPainter* painter, const QRectF& rect, qreal minStepSize, const QColor& color,
								 bool labels);
		QRectF selectedItemsBoundingRect() const;
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

}