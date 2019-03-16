#include "colorpicker.h"

using namespace nap::qt;

Color::Color() : QObject() {}

void Color::setColor(const QColor& col)
{
	if (mColor == col)
		return;
	mColor = col;
	changed(mColor);
}

void Color::setRed(qreal red)
{
	if (red == mColor.redF())
		return;
	mColor.setRedF(red);
	changed(mColor);
}

void Color::setGreen(qreal green)
{
	if (green == mColor.greenF())
		return;
	mColor.setGreenF(green);
	changed(mColor);
}

void Color::setBlue(qreal blue)
{
	if (blue == mColor.blueF())
		return;
	mColor.setBlueF(blue);
	changed(mColor);
}

void Color::setAlpha(qreal alpha)
{
	if (alpha == mColor.alphaF())
		return;
	mColor.setAlphaF(alpha);
	changed(mColor);
}

void Color::setHue(qreal hue)
{
	if (hue == mColor.hueF())
		return;
	mColor.setHsvF(hue, mColor.hsvSaturationF(), mColor.valueF());
	changed(mColor);
}

void Color::setSaturation(qreal saturation)
{
	if (saturation == mColor.saturationF())
		return;
	mColor.setHsvF(mColor.hsvHueF(), saturation, mColor.valueF());
	changed(mColor);
}

void Color::setValue(qreal value)
{
	if (value == mColor.valueF())
		return;
	mColor.setHsvF(mColor.hsvHueF(), mColor.hsvSaturationF(), value);
	changed(mColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GradientSlider::GradientSlider() : QWidget()
{
	mLayout.setContentsMargins(0, 0, 0, 0);
	setLayout(&mLayout);
	mSlider.setOrientation(Qt::Horizontal);
	mLayout.addWidget(&mSlider);

	mSlider.setRange(0, mMaxValue);

	connect(&mSlider, &QSlider::valueChanged, this, &GradientSlider::onSliderChanged);
}

qreal GradientSlider::value() const
{
	return (qreal) mSlider.value() / (qreal) mMaxValue;
}

void GradientSlider::setValue(qreal value)
{
	int intVal = qRound(value * mMaxValue);
	if (mSlider.value() == intVal)
		return;
	mSlider.setValue(intVal);
}

void GradientSlider::onSliderChanged(int v)
{
	changed(value());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ColorSwatch::ColorSwatch() : QWidget()
{
	setMinimumSize(30, 30);
}

void ColorSwatch::setColor(const QColor& col)
{
	mColor = col;
	update();
}

void ColorSwatch::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);
	QPainter painter;
	painter.begin(this);
	painter.setBrush(mColor);
	painter.drawRect(rect());
	painter.end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ColorPicker::ColorPicker() : QWidget()
{
	setLayout(&mLayout);
	int row = 0;

	mLayout.addWidget(&mColorSwatch, row, 0);
	++row;

	mLayout.addWidget(new QLabel("R"), row, 0);
	mLayout.addWidget(&mSliderRed, row, 1);
	++row;

	mLayout.addWidget(new QLabel("G"), row, 0);
	mLayout.addWidget(&mSliderGreen, row, 1);
	++row;

	mLayout.addWidget(new QLabel("B"), row, 0);
	mLayout.addWidget(&mSliderBlue, row, 1);
	++row;

	mLayout.addWidget(new QLabel("A"), row, 0);
	mLayout.addWidget(&mSliderAlpha, row, 1);
	++row;

	mLayout.addWidget(new QLabel("H"), row, 0);
	mLayout.addWidget(&mSliderHue, row, 1);
	++row;

	mLayout.addWidget(new QLabel("S"), row, 0);
	mLayout.addWidget(&mSliderSaturation, row, 1);
	++row;

	mLayout.addWidget(new QLabel("V"), row, 0);
	mLayout.addWidget(&mSliderValue, row, 1);
	++row;

	mLayout.addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), row, 1);

	connect(&mColor, &Color::changed, this, &ColorPicker::onColorChanged);

	connect(&mSliderRed, &GradientSlider::changed, &mColor, &Color::setRed);
	connect(&mSliderGreen, &GradientSlider::changed, &mColor, &Color::setGreen);
	connect(&mSliderBlue, &GradientSlider::changed, &mColor, &Color::setBlue);
	connect(&mSliderAlpha, &GradientSlider::changed, &mColor, &Color::setAlpha);
	connect(&mSliderHue, &GradientSlider::changed, &mColor, &Color::setHue);
	connect(&mSliderSaturation, &GradientSlider::changed, &mColor, &Color::setSaturation);
	connect(&mSliderValue, &GradientSlider::changed, &mColor, &Color::setValue);

	setColor(QColor("#F80"));
}

void ColorPicker::setColor(const QColor& col)
{
	mColor.setColor(col);
}

void ColorPicker::onColorChanged(const QColor col)
{
	mColorSwatch.setColor(col);

	mSliderRed.setValue(mColor.red());
	mSliderGreen.setValue(mColor.green());
	mSliderBlue.setValue(mColor.blue());
	mSliderAlpha.setValue(mColor.alpha());
	mSliderHue.setValue(mColor.hue());
	mSliderSaturation.setValue(mColor.saturation());
	mSliderValue.setValue(mColor.value());
}
