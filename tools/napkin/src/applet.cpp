/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "applet.h"
#include "appletextension.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::Applet)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


namespace napkin
{
	CWDHandle Applet::switchWorkingDir() const
	{
		const auto& applet_ext = getCore().getExtension<AppletExtension>();
		assert(applet_ext.hasProject());
		return applet_ext.switchWorkingDir();
	}

	const nap::ProjectInfo* Applet::getEditorInfo() const
	{
		const auto& applet_ext = getCore().getExtension<AppletExtension>();
		return applet_ext.getProject();
	}
}
