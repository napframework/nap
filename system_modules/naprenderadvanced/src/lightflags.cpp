/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "lightflags.h"
#include "lightcomponent.h"

namespace nap
{
	uint getLightType(LightFlags flags)
	{
		return static_cast<uint>((flags >> 1) & 0x7f);
	}

	bool isShadowSupported(LightFlags flags)
	{
		return static_cast<uint>((flags >> 8) & 0x1) > 0;
	}

	uint getShadowMapType(LightFlags flags)
	{
		return static_cast<uint>((flags >> 16) & 0xff);
	}

	uint getLightIndex(LightFlags flags)
	{
		return static_cast<uint>((flags >> 24) & 0xff);
	}

	LightFlags getLightFlags(const LightComponentInstance& light, uint index)
	{
		LightFlags flags = (static_cast<uint>(light.getLightType()) & 0xff);
		flags |= (static_cast<uint>(light.isShadowSupported()) << 8);
		flags |= (static_cast<uint>(light.getShadowMapType()) & 0xff) << 16;
		flags |= std::min<uint>(index, 0xff) << 24;

		return flags;
	}


	LightEnableFlags NAPAPI getLightEnableFlags(const LightComponentInstance& light)
	{
		LightEnableFlags flags = static_cast<uint>(light.isEnabled());
		flags |= light.isShadowSupported() ? static_cast<uint>(light.isShadowEnabled()) << 1 : 0;
		return flags;
	}
}
