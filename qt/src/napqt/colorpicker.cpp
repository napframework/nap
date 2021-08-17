/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "colorpicker.h"

#include <QtGui>

using namespace nap::qt;

qreal pointLength(const QPointF& p)
{
	return qSqrt(p.x() * p.x() + p.y() * p.y());
}

ColorModel::ColorModel() : QObject() {}

void ColorModel::setColor(const QColor& col)
{
	if (mColor == col)
		return;
	mColor = col;
	changed(mColor);
}

void ColorModel::setRed(qreal r)
{
	if (r == red())
		return;
	mColor.setRedF(r);
	changed(mColor);
}

void ColorModel::setGreen(qreal g)
{
	if (g == green())
		return;
	mColor.setGreenF(g);
	changed(mColor);
}

void ColorModel::setBlue(qreal b)
{
	if (b == blue())
		return;
	mColor.setBlueF(b);
	changed(mColor);
}

void ColorModel::setAlpha(qreal a)
{
	if (a == alpha())
		return;
	mColor.setAlphaF(a);
	changed(mColor);
}

void ColorModel::setHue(qreal h)
{
	if (h == hue())
		return;
	mColor.setHsvF(h, mColor.hsvSaturationF(), mColor.valueF());
	changed(mColor);
}

void ColorModel::setSaturation(qreal s)
{
	if (s == saturation())
		return;
	mColor.setHsvF(mColor.hsvHueF(), s, mColor.valueF());
	changed(mColor);
}

void ColorModel::setValue(qreal v)
{
	if (v == value())
		return;
	mColor.setHsvF(mColor.hsvHueF(), mColor.hsvSaturationF(), v);
	changed(mColor);
}

void ColorModel::setHex(const QString& hexvalue)
{
	auto v = hexvalue;
	if (!v.startsWith('#'))
		v = '#' + v;

	if (v == hex())
		return;
	mColor.setNamedColor(v);
	changed(mColor);
}

QString ColorModel::hex() const
{
	auto s = mColor.name();
	return s.mid(1, s.length() - 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ColorCircle::ColorCircle() : QWidget()
{
	setMinimumSize(300, 300);
}

void ColorCircle::setColor(const QColor& col)
{
	if (mColor == col)
		return;
	blockSignals(true);
	mColor = col;
	update();
	blockSignals(false);
}

QColor ColorCircle::color() const
{
	return mColor;
}

void ColorCircle::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		auto p = toSquarePoint(event->pos(), margin);
		auto len = pointLength(p);
		mClickedInnerCircle = len <= 1.0;

		updateFromPos(event->pos());
		event->accept();
		return;
	}
	QWidget::mousePressEvent(event);
}

void ColorCircle::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		updateFromPos(event->pos());
		event->accept();
		return;
	}
	QWidget::mousePressEvent(event);
}

void ColorCircle::paintEvent(QPaintEvent* event)
{
	QPainter ptr;
	ptr.begin(this);
	ptr.setRenderHint(QPainter::Antialiasing, true);

	// Debug
	ptr.fillRect(rect(), Qt::gray);

	ptr.setPen(Qt::NoPen);

	const auto rec = circleRect();

	QRect cRec = fitSquare(rec);
	qreal cRadius = cRec.height() / 2.0;

	// Color ring
	QConicalGradient grad;
	grad.setCenter(cRec.center());
	grad.setStops({{0.0,       QColor::fromHsvF(0.0, 1, 1)},
				   {0.5 / 3.0, QColor::fromHsvF(0.5 / 3.0, 1, 1)},
				   {1.0 / 3.0, QColor::fromHsvF(1.0 / 3.0, 1, 1)},
				   {1.5 / 3.0, QColor::fromHsvF(1.5 / 3.0, 1, 1)},
				   {2.0 / 3.0, QColor::fromHsvF(2.0 / 3.0, 1, 1)},
				   {2.5 / 3.0, QColor::fromHsvF(2.5 / 3.0, 1, 1)},
				   {1.0,       QColor::fromHsvF(0.0, 1, 1)},
				  });

	ptr.setBrush(grad);
	ptr.drawEllipse(cRec);


	// Inner conical gradient
	cRec.adjust(margin, margin, -margin, -margin);
	qreal val = mColor.valueF();
	grad.setStops({{0.0,       QColor::fromHsvF(0.0, 1, val)},
				   {0.5 / 3.0, QColor::fromHsvF(0.5 / 3.0, 1, val)},
				   {1.0 / 3.0, QColor::fromHsvF(1.0 / 3.0, 1, val)},
				   {1.5 / 3.0, QColor::fromHsvF(1.5 / 3.0, 1, val)},
				   {2.0 / 3.0, QColor::fromHsvF(2.0 / 3.0, 1, val)},
				   {2.5 / 3.0, QColor::fromHsvF(2.5 / 3.0, 1, val)},
				   {1.0,       QColor::fromHsvF(0.0, 1, val)},
				  });
	ptr.setBrush(grad);
	ptr.drawEllipse(cRec);

	ptr.setPen(QPen(palette().window().color()));

	// Transition to center (desaturation)
	QRadialGradient radGrad;
	radGrad.setRadius(cRadius);
	radGrad.setCenter(cRec.center());
	radGrad.setFocalPoint(cRec.center());
	radGrad.setStops({{0, QColor::fromRgbF(val, val, val, 1)},
					  {1, QColor::fromRgbF(val, val, val, 0)}});

	ptr.setBrush(radGrad);
	ptr.drawEllipse(cRec);

	// Draw color handle
	{
		qreal r = cRadius - margin / 2.0;
		qreal a = -mColor.hsvHueF() * M_PI * 2;
		qreal px = cRec.center().x() + qCos(a) * r;
		qreal py = cRec.center().y() + qSin(a) * r;
		QRectF hRec(px - mHandleRadius, py - mHandleRadius, mHandleRadius * 2, mHandleRadius * 2);
		ptr.setBrush(Qt::NoBrush);
		ptr.setPen(QPen(Qt::black, 4));
		ptr.drawEllipse(hRec);
		ptr.setPen(QPen(Qt::white, 2));
		ptr.drawEllipse(hRec);
	}

	// Draw center color handle
	{
		qreal r = (cRadius - margin - margin / 2.0) * mColor.saturationF();
		qreal a = -mColor.hsvHueF() * M_PI * 2;
		qreal px = cRec.center().x() + qCos(a) * r;
		qreal py = cRec.center().y() + qSin(a) * r;
		QRectF hRec(px - mHandleRadius, py - mHandleRadius, mHandleRadius * 2, mHandleRadius * 2);
		ptr.setBrush(Qt::NoBrush);
		ptr.setPen(QPen(Qt::black, 4));
		ptr.drawEllipse(hRec);
		ptr.setPen(QPen(Qt::white, 2));
		ptr.drawEllipse(hRec);
	}

	ptr.end();
}

void ColorCircle::updateFromPos(const QPoint& point)
{
	auto p = toSquarePoint(point, margin + (mHandleRadius* 2));
	qreal angle = -qAtan2(p.y(), p.x()) + M_PI * 2.0;
	qreal hue = fmod(angle / M_PI / 2.0, 1.0);
	qreal sat = pointLength(p);

	QColor col;
	if (mClickedInnerCircle)
		col.setHsvF(qBound(0.0, hue, 1.0), qBound(0.0, sat, 1.0), mColor.valueF());
	else
		col.setHsvF(qBound(0.0, hue, 1.0), mColor.saturationF(), mColor.valueF());

	valueChanged(col);
}

QRect ColorCircle::fitSquare(const QRect& rec)
{
	const qreal ratio = (qreal) rec.width() / (qreal) rec.height();
	auto cRec = rec;
	if (ratio > 1)
	{
		cRec.setX(rec.left() + (rec.width() / 2 - rec.height() / 2));
		cRec.setWidth(rec.height());
	}
	else
	{
		cRec.setY(rec.top() + (rec.height() / 2 - rec.width() / 2));
		cRec.setHeight(rec.width());
	}
	return cRec;
}

QPointF ColorCircle::toSquarePoint(const QPoint& p, int margin)
{
	auto rec = circleRect();
	const qreal ratio = (qreal) rec.width() / (qreal) rec.height();
	qreal leftMargin = 0;
	qreal topMargin = 0;
	qreal side;
	if (ratio > 1)
	{
		side = rec.height();
		leftMargin = (rec.width() - side) / 2.0;
	}
	else
	{
		side = rec.width();
		topMargin = (rec.height() - side) / 2.0;
	}
	qreal halfSide = side / 2.0;
	qreal x = p.x() - halfSide - leftMargin;
	qreal y = p.y() - halfSide - topMargin;
	qreal hsm = halfSide - margin;
	return {x / hsm, y / hsm};
}

QRect ColorCircle::circleRect() const
{
	return rect().adjusted(0, 0, -1, -1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GradientSlider::GradientSlider() : QWidget()
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	mGradient = QLinearGradient(0, 0, 1, 0);
	mGradient.setCoordinateMode(QGradient::StretchToDeviceMode);
	setGradientStops({{0, Qt::red},
					  {1, Qt::yellow}});
}

void GradientSlider::setGradientStops(const QGradientStops& stops)
{
	mGradient.setStops(stops);
}

void GradientSlider::setValue(qreal v)
{
	if (mValue == v)
		return;
	mValue = v;
	update();
	valueChanged(mValue);
}

qreal GradientSlider::value() const
{
	return mValue;
}

void GradientSlider::paintEvent(QPaintEvent* event)
{
	QPainter ptr;
	ptr.begin(this);

	const auto arec = hotArea();

//	// debug
//	ptr.fillRect(rect(), Qt::magenta);

	// draw gradient box
	QBrush b(mGradient);
	ptr.fillRect(arec, b);
	ptr.setPen(QPen(Qt::black));
	ptr.drawRect(hotArea());

	// draw cursor
	int intVal = qFloor(mValue * hotArea().width());
	QRect cursorRect(intVal, rect().top(), mCursorWidth, rect().height() - 1);
	ptr.fillRect(cursorRect, Qt::white);
	ptr.drawRect(cursorRect);

	ptr.end();
}
QSize GradientSlider::sizeHint() const
{
	return {10, 10};
}

void GradientSlider::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		updateCursor(event->pos());
		event->accept();
		return;
	}
	QWidget::mousePressEvent(event);
}

void GradientSlider::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		updateCursor(event->pos());
		event->accept();
		return;
	}
	QWidget::mouseMoveEvent(event);
}

void GradientSlider::updateCursor(QPoint mousePos)
{

	int lm = leftMargin();
	int x = mousePos.x() - lm;
	qreal value = qBound(0.0, (qreal) x / (qreal) hotArea().width(), 1.0);
	setValue(value);
	update();
}

QRectF GradientSlider::hotArea() const
{
	return rect().adjusted(leftMargin(), mVertMargin, -rightMargin() - 1, -mVertMargin - mVertMargin + 1);
}

int GradientSlider::leftMargin() const
{
	return qFloor(mCursorWidth / 2.0);
}

int GradientSlider::rightMargin() const
{
	return qCeil(mCursorWidth / 2.0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ChannelInput::ChannelInput() : QWidget()
{
	mLayout.setContentsMargins(0, 0, 0, 0);
	setLayout(&mLayout);
	mLayout.addWidget(&mSpinBox);
	mLayout.addWidget(&mSlider);

	mSpinBox.setSingleStep(0.01);

	connect(&mSlider, &GradientSlider::valueChanged, this, &ChannelInput::onSliderChanged);
	connect(&mSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &ChannelInput::onSpinboxChanged);
}

qreal ChannelInput::value() const
{
	return mValue;
}

void ChannelInput::setValue(qreal value)
{
	if (mValue == value)
		return;

	mValue = value;

	mSlider.blockSignals(true);
	mSlider.setValue(mValue);
	mSlider.blockSignals(false);

	mSpinBox.blockSignals(true);
	mSpinBox.setValue(mValue);
	mSpinBox.blockSignals(false);

	changed(mValue);
}

void ChannelInput::setGradientStops(const QGradientStops& stops)
{
	mSlider.setGradientStops(stops);
	update();
}

void ChannelInput::onSliderChanged(qreal v)
{
	setValue(v);
}

void ChannelInput::onSpinboxChanged(double v)
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
	mLayout.addWidget(&mColorCircle, row, 1);
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
	mSliderAlpha.setGradientStops({{0, Qt::black},
								   {1, Qt::white}});
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

	connect(&mColor, &ColorModel::changed, this, &ColorPicker::onColorChanged);

	connect(&mColorCircle, &ColorCircle::valueChanged, &mColor, &ColorModel::setColor);
	connect(&mSliderRed, &ChannelInput::changed, &mColor, &ColorModel::setRed);
	connect(&mSliderGreen, &ChannelInput::changed, &mColor, &ColorModel::setGreen);
	connect(&mSliderBlue, &ChannelInput::changed, &mColor, &ColorModel::setBlue);
	connect(&mSliderAlpha, &ChannelInput::changed, &mColor, &ColorModel::setAlpha);
	connect(&mSliderHue, &ChannelInput::changed, &mColor, &ColorModel::setHue);
	connect(&mSliderSaturation, &ChannelInput::changed, &mColor, &ColorModel::setSaturation);
	connect(&mSliderValue, &ChannelInput::changed, &mColor, &ColorModel::setValue);
	connect(&mHexEdit, &QLineEdit::editingFinished, [this]() { mColor.setHex(mHexEdit.text().trimmed()); });

	setColor(QColor("#F80"));
}

QColor ColorPicker::color() const
{
	return mColor.color();
}

void ColorPicker::setColor(const QColor& col)
{
	if (mColor.color() == col)
		return;
	mColor.setColor(col);
	colorChanged();
}

void ColorPicker::onColorChanged(const QColor& col)
{
	mColorSwatch.setColor(col);

	mColorCircle.blockSignals(true);
	mSliderRed.blockSignals(true);
	mSliderGreen.blockSignals(true);
	mSliderBlue.blockSignals(true);
	mSliderAlpha.blockSignals(true);
	mSliderHue.blockSignals(true);
	mSliderSaturation.blockSignals(true);
	mSliderValue.blockSignals(true);

	mColorCircle.setColor(mColor.color());
	mSliderRed.setValue(mColor.red());
	mSliderGreen.setValue(mColor.green());
	mSliderBlue.setValue(mColor.blue());
	mSliderAlpha.setValue(mColor.alpha());
	mSliderHue.setValue(mColor.hue());
	mSliderSaturation.setValue(mColor.saturation());
	mSliderValue.setValue(mColor.value());

	mColorCircle.blockSignals(false);
	mSliderRed.blockSignals(false);
	mSliderGreen.blockSignals(false);
	mSliderBlue.blockSignals(false);
	mSliderAlpha.blockSignals(false);
	mSliderHue.blockSignals(false);
	mSliderSaturation.blockSignals(false);
	mSliderValue.blockSignals(false);

	mSliderRed.setGradientStops({{0, QColor::fromRgbF(0, mColor.green(), mColor.blue())},
								 {1, QColor::fromRgbF(1, mColor.green(), mColor.blue())}});

	mSliderGreen.setGradientStops({{0, QColor::fromRgbF(mColor.red(), 0, mColor.blue())},
								   {1, QColor::fromRgbF(mColor.red(), 1, mColor.blue())}});

	mSliderBlue.setGradientStops({{0, QColor::fromRgbF(mColor.red(), mColor.green(), 0)},
								  {1, QColor::fromRgbF(mColor.red(), mColor.green(), 1)}});


	mSliderHue.setGradientStops({{0.0,       QColor::fromHsvF(0.0, mColor.saturation(), mColor.value())},
								 {0.5 / 3.0, QColor::fromHsvF(0.5 / 3.0, mColor.saturation(), mColor.value())},
								 {1.0 / 3.0, QColor::fromHsvF(1.0 / 3.0, mColor.saturation(), mColor.value())},
								 {1.5 / 3.0, QColor::fromHsvF(1.5 / 3.0, mColor.saturation(), mColor.value())},
								 {2.0 / 3.0, QColor::fromHsvF(2.0 / 3.0, mColor.saturation(), mColor.value())},
								 {2.5 / 3.0, QColor::fromHsvF(2.5 / 3.0, mColor.saturation(), mColor.value())},
								 {1.0,       QColor::fromHsvF(0.0, mColor.saturation(), mColor.value())}});

	mSliderSaturation.setGradientStops({{0, QColor::fromHsvF(mColor.hue(), 0, mColor.value())},
										{1, QColor::fromHsvF(mColor.hue(), 1, mColor.value())}});

	mSliderValue.setGradientStops({{0, QColor::fromHsvF(mColor.hue(), mColor.saturation(), 0)},
								   {1, QColor::fromHsvF(mColor.hue(), mColor.saturation(), 1)}});

	mHexEdit.setText(mColor.hex());
}


nap::qt::ColorPickerDialog::ColorPickerDialog(QWidget* parent) : QDialog(parent)
{
	setWindowTitle("Select Color");
	setLayout(&mLayout);
	mLayout.addWidget(&mColorPicker);
	setWindowModality(Qt::ApplicationModal);
}
