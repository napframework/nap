/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QPainter>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDialog>

namespace nap
{

	namespace qt
	{
		/**
		 * The internal color model used by the color picker.
		 */
		class ColorModel : public QObject
		{
		Q_OBJECT
		public:
			ColorModel();

			void setColor(const QColor& col);
			QColor color() const { return mColor; }

			qreal red() const { return mColor.redF(); }
			void setRed(qreal r);
			qreal green() const { return mColor.greenF(); }
			void setGreen(qreal green);
			qreal blue() const { return mColor.blueF(); }
			void setBlue(qreal b);
			qreal alpha() const { return mColor.alphaF(); }
			void setAlpha(qreal a);
			qreal hue() const { return mColor.hsvHueF(); }
			void setHue(qreal hue);
			qreal saturation() const { return mColor.hsvSaturationF(); }
			void setSaturation(qreal saturation);
			qreal value() const { return mColor.valueF(); }
			void setValue(qreal v);

			void setHex(const QString& hexvalue);
			QString hex() const;

		Q_SIGNALS:
			void changed(QColor col);

		private:
			QColor mColor;
		};

		/**
		 * The color wheel used by the picker.
		 */
		class ColorCircle : public QWidget
		{
		Q_OBJECT
		public:
			ColorCircle();

			void setColor(const QColor& col);
			QColor color() const;
            void mousePressEvent(QMouseEvent* event) override;
            void mouseMoveEvent(QMouseEvent* event) override;

        Q_SIGNALS:
		    void valueChanged(QColor color);

		protected:
			void paintEvent(QPaintEvent* event) override;

		private:
		    void updateFromPos(const QPoint& p);
			static QRect fitSquare(const QRect& rec) ;
			QPointF toSquarePoint(const QPoint& p, int margin);
			QRect circleRect() const;

			int margin = 20;
			int mHandleRadius = 6;
			bool mClickedInnerCircle = false;
			QColor mColor;
		};

		/**
		 * A slider widget that displays a color gradient and allows interaction with the slider head.
		 */
		class GradientSlider : public QWidget
		{
		Q_OBJECT
		public:
			GradientSlider();

			void setGradientStops(const QGradientStops& stops);
			void setValue(qreal v);
			qreal value() const;

			QSize sizeHint() const override;

		Q_SIGNALS:
			void valueChanged(qreal value);

		protected:
			void mousePressEvent(QMouseEvent* event) override;
			void mouseMoveEvent(QMouseEvent* event) override;
			void paintEvent(QPaintEvent* event) override;

		private:
			void updateCursor(QPoint mousePos);
			QRectF hotArea() const;
			int leftMargin() const;
			int rightMargin() const;
			int mCursorWidth = 4;
			int mVertMargin = 2;
			qreal mValue = 0;
			QGradient mGradient;
		};

		/**
		 * A composite widget containing a GradientSlider and a spinbox to input values numerically.
		 */
		class ChannelInput : public QWidget
		{
		Q_OBJECT
		public:
			ChannelInput();
			qreal value() const;
			void setValue(qreal value);
			void setGradientStops(const QGradientStops& stops);

		Q_SIGNALS:
			void changed(qreal value);

		private:
			void onSliderChanged(qreal value);
			void onSpinboxChanged(double v);

			qreal mValue = 0;
			QHBoxLayout mLayout;
			QDoubleSpinBox mSpinBox;
			GradientSlider mSlider;
		};

		/**
		 * A simple rectangle displaying a single color
		 */
		class ColorSwatch : public QWidget
		{
		public:
			ColorSwatch();
			void setColor(const QColor& col);
			QColor color() const { return mColor; }

		protected:
			void paintEvent(QPaintEvent* event) override;

		private:
			QColor mColor;
		};

		/**
		 * The entire color picker composite widget with a color wheel and several sliders.
		 */
		class ColorPicker : public QWidget
		{
		Q_OBJECT
		public:
			ColorPicker();

			QColor color() const;
			void setColor(const QColor& col);

		Q_SIGNALS:
			void colorChanged();

		private:

			void onColorChanged(const QColor& col);

			QGridLayout mLayout;

			ColorModel mColor;

			ColorSwatch mColorSwatch;
			ColorCircle mColorCircle;

			ChannelInput mSliderRed;
			ChannelInput mSliderGreen;
			ChannelInput mSliderBlue;
			ChannelInput mSliderAlpha;

			ChannelInput mSliderHue;
			ChannelInput mSliderSaturation;
			ChannelInput mSliderValue;

			QLineEdit mHexEdit;
		};

		/**
		 * Color picker dialog
		 */
		class ColorPickerDialog : public QDialog
		{
			Q_OBJECT
		public:
			explicit ColorPickerDialog(QWidget* parent);

			explicit ColorPickerDialog(QWidget* parent, const QColor& color);

			void init();

			/**
			 * @return selected color
			 */
			QColor getColor() const;

		private:
			QVBoxLayout mLayout;
			ColorPicker mColorPicker;
		};
	}
}
