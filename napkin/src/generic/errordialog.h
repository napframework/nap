#pragma once

#include <QDialog>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include <nap/logger.h>

/**
 * A dialog for displaying an annoying error message to the user.
 */
class ErrorDialog : public QDialog
{
	Q_OBJECT
public:
	ErrorDialog(QWidget* parent);

	/**
	 * Add a message to the dialog
	 * @param message
	 */
	void addMessage(nap::LogMessage message);

	/**
	 * Overridden to set a reasonable size
	 * @return Preferred size of the error dialog
	 */
	QSize sizeHint() const override;

protected:
	void closeEvent(QCloseEvent* event) override;
	void showEvent(QShowEvent* event) override;

private:
	QVBoxLayout mLayout;
	QTextBrowser mText;
	QDialogButtonBox mButtonBox;
};