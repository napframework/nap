/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>

#include <QDialog>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QVBoxLayout>

namespace nap
{

	namespace qt
	{
		/**
		 * A dialog for displaying an annoying error message to the user.
		 * You can show this dialog yourself or use the static convenience method showMessage(),
		 * when using the latter method, make sure to attach to dialog to a window using setDefaultParent()
		 */
		class ErrorDialog : public QDialog
		{
		Q_OBJECT
		public:
			explicit ErrorDialog(QWidget* parent);

			/**
			 * Add a message to the dialog
			 * @param message
			 */
			void addMessage(const QString& message);

			/**
			 * When set to true, all messages will be removed from this dialog upon close.
			 */
			void setClearOnClose(bool b) { mClearOnClose = b; }

			/**
			 * When true, this dialog will remove all messages when closing.
			 */
			bool isClearOnClose() const { return mClearOnClose; }

			/**
			 * Overridden to set a reasonable size
			 * @return Preferred size of the error dialog
			 */
			QSize sizeHint() const override;

			/**
			 * Display the error dialog and show the provided message.
			 * @param message The message to be shown (or added) to the error dialog
			 */
			static void showMessage(const QString& message);

			/**
			 * When using the static showMessage(QString), this parent will be used to attach the dialog to.
			 * @param parent Most likely the main window
			 */
			static void setDefaultParent(QWidget* parent);

		protected:
			void closeEvent(QCloseEvent* event) override;
			void showEvent(QShowEvent* event) override;

		private:
			static ErrorDialog& instance();

			QVBoxLayout mLayout;
			QTextBrowser mText;
			QDialogButtonBox mButtonBox;
			bool mClearOnClose = true;
			static QWidget* mParent;
			static std::shared_ptr<ErrorDialog> mInstance; // used when using static methods
		};

	} // namespace qt

} // namespace nap
