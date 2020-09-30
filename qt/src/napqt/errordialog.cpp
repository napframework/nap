/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "errordialog.h"
#include <QPushButton>

using namespace nap::qt;

QWidget* ErrorDialog::mParent = nullptr;
std::shared_ptr<ErrorDialog> ErrorDialog::mInstance = nullptr;

void ErrorDialog::setDefaultParent(QWidget* parent)
{
	mParent = parent;
}


ErrorDialog::ErrorDialog(QWidget* parent) : QDialog(parent), mButtonBox(QDialogButtonBox::Close)
{
	setWindowTitle("Error");
	setLayout(&mLayout);

	setWindowFlag(Qt::WindowCloseButtonHint, false);

	mLayout.addWidget(&mText);
	mLayout.addWidget(&mButtonBox);

	setWindowModality(Qt::ApplicationModal);
	mText.setAutoFillBackground(false);

	auto btClose = mButtonBox.button(QDialogButtonBox::Close);
	connect(btClose, &QPushButton::clicked, [this]() {
		close();
	});
}

void ErrorDialog::addMessage(const QString& message)
{
	mText.append(message);
}

void ErrorDialog::closeEvent(QCloseEvent* event)
{
	if (mClearOnClose)
		mText.clear();
	QDialog::closeEvent(event);
}

QSize ErrorDialog::sizeHint() const
{
	return {600, 300};
}

void ErrorDialog::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);
}

void ErrorDialog::showMessage(const QString& message)
{
	ErrorDialog& dialog = instance();
	dialog.addMessage(message);
	if (!dialog.isVisible())
		dialog.show();
}

ErrorDialog& ErrorDialog::instance()
{
	if (mInstance == nullptr)
	{
		mInstance = std::make_shared<ErrorDialog>(mParent);
	}
	return *mInstance;
}

