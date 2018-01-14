#pragma once

#include <QtWidgets/QGraphicsView>

namespace napkin {


	class GridView : public QGraphicsView {
		enum ZoomMode {
			IgnoreAspectRatio, KeepAspectRatio, Horizontal, Vertical
		};

	public:
		GridView();
		void pan(const QPointF& delta);
		void zoom(const QPointF& delta, const QPointF& pivot);
		void centerView();

	protected:
		void drawBackground(QPainter* painter, const QRectF& rect) override;

		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void wheelEvent(QWheelEvent* event) override;
		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;

	private:

		void applyViewTransform();
		const QPointF viewScale() const;
		const QPointF viewPos() const;

		QPointF mMousePressPos;
		QPointF mMouseLastPos;
		QTransform mViewTransform;
		QSize mViewSize;
		QFont mRulerFont;
		ZoomMode mZoomMode = IgnoreAspectRatio;

		qreal calcGridStep(qreal desiredSpacing, qreal viewWidth, qreal sceneRectWidth) const;
	};

}