#pragma once

#include <QDoubleSpinBox>
#include <QStyle>
#include <QPainter>
#include <QLineEdit>

namespace napqt
{

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

}