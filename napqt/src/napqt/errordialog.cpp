#include "errordialog.h"

#include <QPushButton>



napqt::ErrorDialog::ErrorDialog(QWidget* parent) : QDialog(parent), mButtonBox(QDialogButtonBox::Close)
{
	setWindowTitle("Error");
	setLayout(&mLayout);

	setWindowFlag(Qt::WindowCloseButtonHint, false);

	mLayout.addWidget(&mText);
	mLayout.addWidget(&mButtonBox);

	setWindowModality(Qt::ApplicationModal);

	auto btClose = mButtonBox.button(QDialogButtonBox::Close);
	connect(btClose, &QPushButton::clicked, [this]() {
		close();
	});
}

void napqt::ErrorDialog::addMessage(const QString& message)
{
	mText.append(message + "\n");
}

void napqt::ErrorDialog::closeEvent(QCloseEvent* event)
{
	mText.clear();
	QDialog::closeEvent(event);
}

QSize napqt::ErrorDialog::sizeHint() const
{
	return {600, 300};
}

void napqt::ErrorDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);
}

