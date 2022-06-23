/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "rttiitem.h"

#include <QFileDialog>
#include <QStandardItem>
#include <rtti/typeinfo.h>

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
