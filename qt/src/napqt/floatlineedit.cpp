#include "floatlineedit.h"

napqt::FloatLineEdit::FloatLineEdit(QWidget* parent) : QLineEdit(parent)
{
	setValidator(&mDoubleValidator);
	connect(this, &QLineEdit::editingFinished, [this]() {
		valueChanged(value());
	});
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
}

void napqt::FloatLineEdit::setIndeterminate(bool b) {
	mIsIntederminate = b;
	if (mIsIntederminate)
		setText("-");
	else
		setText(QString::number(value()));
	update();
}

void napqt::FloatLineEdit::setValue(qreal value) {
	mValue = value;
	if (!mIsIntederminate)
		setText(QString::number(value));
}

qreal napqt::FloatLineEdit::value() const {
	bool ok;
	qreal val = text().toDouble(&ok);
	if (!ok)
		val = mValue;
	return val;
}
