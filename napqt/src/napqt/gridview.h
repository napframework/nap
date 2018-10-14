#pragma once

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QRubberBand>

namespace napqt {


	class GridView : public QGraphicsView {
	Q_OBJECT

	public:
		enum class ZoomMode {
			IgnoreAspectRatio, KeepAspectRatio, Horizontal, Vertical
		};

		enum class PanMode {
			Horizontal, Vertical, Parallax
		};

		enum class RulerFormat {
			Float, SMPTE
		};

		explicit GridView(QWidget* parent=nullptr);
		~GridView() {}

		void setPanZoomMode(PanMode panMode, ZoomMode zoomMode) { mPanMode=panMode; mZoomMode = zoomMode; }
		void setFramePanZoomMode(PanMode panMode, ZoomMode zoomMode) { mFramePanMode=panMode; mFrameZoomMode = zoomMode; }

		void pan(const QPointF& delta);
		void zoom(const QPointF& delta, const QPointF& pivot);
		void centerView();
		void frameAll(QMargins margins);
		void frameSelected(QMargins margins);
		void frameView(const QRectF& rect, QMargins margins);
		void fitInView(const QRectF& rect, const QMargins& margins, bool horizontal, bool vertical);
		void setVerticalScroll(int value);
		void setGridEnabled(bool enabled);
		void setPanBounds(qreal left, qreal top, qreal right, qreal bottom);
		void setPanBounds(const QRectF& rec);
		void constrainTransform(QTransform& xf);
		void constrainView();

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
		QRectF selectedItemsBoundingRect() const;
		const QPointF viewScale() const;
		const QPointF viewPos() const;


		QPoint mMouseLastPos;
		QPoint mMouseDelta;
		QSize mViewSize;
		QFont mRulerFont;
		QRubberBand mRubberBand;
		bool mGridEnabled = true;

		ZoomMode mZoomMode = ZoomMode::Horizontal;
		PanMode mPanMode = PanMode::Parallax;
		ZoomMode mFrameZoomMode = ZoomMode::Horizontal;
		PanMode mFramePanMode = PanMode::Parallax;

		QRectF mPanBounds;
		RulerFormat mRulerFormat = RulerFormat::Float;
		qreal mFramerate = 30;

		qreal calcGridStep(qreal desiredSpacing, qreal viewWidth, qreal sceneRectWidth) const;
		qreal calcGridStepTime(qreal desiredSpacing, qreal viewWidth, qreal sceneRectWidth, qreal minStepSize) const;
	};

}