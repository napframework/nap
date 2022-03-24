/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "napkinutils.h"

QString napkin::napkinutils::getOpenFilename(QWidget* parent, const QString& caption, const QString& dir, const QString& filter)
{
	return QFileDialog::getOpenFileName(parent, caption, dir, filter, nullptr, QFileDialog::DontUseNativeDialog);
}
