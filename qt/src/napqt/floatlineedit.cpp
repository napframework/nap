/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "floatlineedit.h"

using namespace nap::qt;

FloatLineEdit::FloatLineEdit(QWidget* parent) : QLineEdit(parent)
{
	setValidator(&mDoubleValidator);
	connect(this, &QLineEdit::editingFinished, [this]() {
		valueChanged(value());
	});
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
}

void FloatLineEdit::setIndeterminate(bool b) {
	mIsIntederminate = b;
	if (mIsIntederminate)
		setText("-");
	else
		setText(QString::number(value()));
	update();
}

void FloatLineEdit::setValue(qreal value) {
	mValue = value;
	if (!mIsIntederminate)
		setText(QString::number(value));
}

qreal FloatLineEdit::value() const {
	bool ok;
	qreal val = text().toDouble(&ok);
	if (!ok)
		val = mValue;
	return val;
}
