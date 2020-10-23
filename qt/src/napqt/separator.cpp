/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "separator.h"

nap::qt::Separator::Separator(Qt::Orientation orientation) : QFrame()
{
	if (orientation == Qt::Horizontal)
		setFrameShape(QFrame::HLine);
	else if (orientation == Qt::Vertical)
		setFrameShape(QFrame::VLine);
	setFrameShadow(QFrame::Sunken);
}
