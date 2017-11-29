#pragma once

#include <QLineEdit>
#include <QToolButton>
#include <QWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
namespace napkin
{
	/**
	 * A widget displaying and allowing to change a filename using a textfield and a browse button.
	 */
	class FileSelector : public QWidget
	{
		Q_OBJECT
	public:
		FileSelector();

		/**
		 * Set the file filter to be used in the file selection dialog.
		 * @param filter The file filter in the format Qt accepts.
		 */
		void setFileFilter(const QString& filter);

		/**
		 * Set the currently displayed filename, but don't send a signal
		 * @param filename The absolute or relative filename
		 */
		void setFilename(const QString& filename);

		/**
		 * @return The currently displayed filename
		 */
		const QString getFilename();

	Q_SIGNALS:

		/**
		 * Emit when the user has changed the filename by manually editing or using the file open dialog.
		 * @param filename The newly set filename.
		 */
		void filenameChanged(const QString& filename);

	private:
		void onEditingFinished();

		void onBrowseButtonClicked();

		QHBoxLayout mLayout; // Layout
		QLineEdit mLineEdit; // Shows the filename
		QToolButton mBrowseButton; // Click and browse
		QString mFileFilter; // File filter used by the dialog
	};
};