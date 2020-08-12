#include "napkinutils.h"

QString napkin::napkinutils::getOpenFilename(QWidget* parent, const QString& caption, const QString& dir, const QString& filter)
{
	return QFileDialog::getOpenFileName(parent, caption, dir, filter, nullptr);
}
