
#include "colorpicker.h"

#include <QtGui>

using namespace nap::qt;

Color::Color() : QObject() {}

void Color::setColor(const QColor& col)
{
	if (mColor == col)
		return;
	mColor = col;
	changed(mColor);
}

void Color::setRed(qreal r)
{
	if (r == red())
		return;
	mColor.setRedF(r);
	changed(mColor);
}

void Color::setGreen(qreal g)
{
	if (g == green())
		return;
	mColor.setGreenF(g);
	changed(mColor);
}

void Color::setBlue(qreal b)
{
	if (b == blue())
		return;
	mColor.setBlueF(b);
	changed(mColor);
}

void Color::setAlpha(qreal a)
{
	if (a == alpha())
		return;
	mColor.setAlphaF(a);
	changed(mColor);
}

void Color::setHue(qreal h)
{
	if (h == hue())
		return;
	mColor.setHsvF(h, mColor.hsvSaturationF(), mColor.valueF());
	changed(mColor);
}

void Color::setSaturation(qreal s)
{
	if (s == saturation())
		return;
	mColor.setHsvF(mColor.hsvHueF(), s, mColor.valueF());
	changed(mColor);
}

void Color::setValue(qreal v)
{
	if (v == value())
		return;
	mColor.setHsvF(mColor.hsvHueF(), mColor.hsvSaturationF(), v);
	changed(mColor);
}

void Color::setHex(const QString& hexvalue)
{
	auto v = hexvalue;
	if (!v.startsWith('#'))
		v = '#' + v;

	if (v == hex())
		return;
	mColor.setNamedColor(v);
	changed(mColor);
}

QString Color::hex() const
{
	auto s = mColor.name();
	return s.mid(1, s.length() - 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GradientSlider::GradientSlider() : QWidget()
{
	mLayout.setContentsMargins(0, 0, 0, 0);
	setLayout(&mLayout);
	mLayout.addWidget(&mSpinBox);
	mLayout.addWidget(&mSlider);

	mSlider.setOrientation(Qt::Horizontal);
	mSlider.setRange(0, mMaxValue);

	mSpinBox.setSingleStep(0.01);

	connect(&mSlider, &QSlider::valueChanged, this, &GradientSlider::onSliderChanged);
	connect(&mSpinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &GradientSlider::onSpinboxChanged);
}

qreal GradientSlider::value() const
{
	return mValue;
}

void GradientSlider::setValue(qreal value)
{
	if (mValue == value)
		return;

	mValue = value;

	mSlider.blockSignals(true);
	mSlider.setValue(qRound(mValue * mMaxValue));
	mSlider.blockSignals(false);

	mSpinBox.blockSignals(true);
	mSpinBox.setValue(mValue);
	mSpinBox.blockSignals(false);

	changed(mValue);
}

void GradientSlider::onSliderChanged(int v)
{
	setValue((qreal) mSlider.value() / (qreal) mMaxValue);
}

void GradientSlider::onSpinboxChanged(double v)
{
	setValue(v);
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

void ColorSwatch::paintEvent(QPaintEvent* event)
{
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
	mLayout.setHorizontalSpacing(2);
	mLayout.setVerticalSpacing(2);

	auto font = mHexEdit.font();
	font.setFamily("monospace");
	mHexEdit.setFont(font);

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

	mLayout.addWidget(new QLabel("Hex"), row, 0);
	mLayout.addWidget(&mHexEdit, row, 1);
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
	connect(&mHexEdit, &QLineEdit::editingFinished, [this]() { mColor.setHex(mHexEdit.text().trimmed()); });

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

	mHexEdit.setText(mColor.hex());
}
