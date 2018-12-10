#include "errordialog.h"

#include <QPushButton>

using namespace napqt;

QWidget* ErrorDialog::mParent = nullptr;
std::shared_ptr<ErrorDialog> ErrorDialog::mInstance = nullptr;

void ErrorDialog::setDefaultParent(QWidget* parent)
{
	mParent = parent;
}


napqt::ErrorDialog::ErrorDialog(QWidget* parent) : QDialog(parent), mButtonBox(QDialogButtonBox::Close)
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

void napqt::ErrorDialog::addMessage(const QString& message)
{
	mText.append(message);
}

void napqt::ErrorDialog::closeEvent(QCloseEvent* event)
{
	if (mClearOnClose)
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

void napqt::ErrorDialog::showMessage(const QString& message)
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

