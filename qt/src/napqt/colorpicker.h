#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QPainter>

namespace nap
{

	namespace qt
	{

		class Color : public QObject
		{
		Q_OBJECT
		public:
			Color();

			void setColor(const QColor& col);
			QColor color() const { return mColor; }

			qreal red() const { return mColor.redF(); }
			void setRed(qreal red);
			qreal green() const { return mColor.greenF(); }
			void setGreen(qreal green);
			qreal blue() const { return mColor.blueF(); }
			void setBlue(qreal blue);
			qreal alpha() const { return mColor.alphaF(); }
			void setAlpha(qreal alpha);
			qreal hue() const { return mColor.hsvHueF(); }
			void setHue(qreal hue);
			qreal saturation() const { return mColor.hsvSaturationF(); }
			void setSaturation(qreal saturation);
			qreal value() const { return mColor.valueF(); }
			void setValue(qreal value);

		Q_SIGNALS:
			void changed(QColor col);

		private:
			QColor mColor;
		};

		class GradientSlider : public QWidget
		{
			Q_OBJECT
		public:
			GradientSlider();
			qreal value() const;
			void setValue(qreal value);

		Q_SIGNALS:
			void changed(qreal value);

		private:
			void onSliderChanged(int value);

			QHBoxLayout mLayout;
			QSlider mSlider;
			const int mMaxValue = 2048; // WARNING: this has effect on the apparent precision of the slider
		};

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

		class ColorPicker : public QWidget
		{
		Q_OBJECT
		public:
			ColorPicker();

		QColor color() const { return mColor.color(); }
		void setColor(const QColor& col);

		Q_SIGNALS:
			void colorChanged();

		private:

			void onColorChanged(QColor col);

			QGridLayout mLayout;

			Color mColor;

			ColorSwatch mColorSwatch;
			GradientSlider mSliderRed;
			GradientSlider mSliderGreen;
			GradientSlider mSliderBlue;
			GradientSlider mSliderAlpha;

			GradientSlider mSliderHue;
			GradientSlider mSliderSaturation;
			GradientSlider mSliderValue;

		};
	}
}