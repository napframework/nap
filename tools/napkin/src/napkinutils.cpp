#include "napkinutils.h"

QString napkin::napkinutils::getOpenFilename(QWidget* parent, const QString& caption, const QString& dir, const QString& filter)
{
	// Work around a crash on windows 10 when the dialog is opened twice
	// TODO: Find out why the crash happens and if there's a fix, ux is better when the native dialog is used.
	QFileDialog::Options options;
#ifdef _WIN32
	options = QFileDialog::DontUseNativeDialog;
#endif
	return QFileDialog::getOpenFileName(parent, caption, dir, filter, nullptr, options);
}
