/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QDoubleSpinBox>
#include <QStyle>
#include <QPainter>
#include <QLineEdit>

namespace nap
{
	namespace qt
	{

		/**
		 * A QLineEdit, extended to accept only qreal values and the ability to set a placeholder text.
		 * This is mainly used for multi-editing float values in a single field.
		 * When multiple different values are edited, setIndeterminate() can be used to indicate they're different.
		 */
		class FloatLineEdit : public QLineEdit
		{
		Q_OBJECT
		public:
			FloatLineEdit(QWidget* parent = nullptr);

			void setIndeterminate(bool b);
			void setValue(qreal value);
			qreal value() const;

		Q_SIGNALS:
			void valueChanged(qreal value);

		private:
			bool mIsIntederminate = false;
			qreal mValue = 0;
			QDoubleValidator mDoubleValidator;
		};

	} // namespace qt

} // namespace nap

