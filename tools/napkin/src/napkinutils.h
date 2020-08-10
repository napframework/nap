#pragma once

#include <QFileDialog>

namespace napkin
{
	namespace napkinutils
	{
		QString getOpenFilename(QWidget *parent = nullptr,
								const QString &caption = {},
								const QString &dir = {},
								const QString &filter = {});
	}
}