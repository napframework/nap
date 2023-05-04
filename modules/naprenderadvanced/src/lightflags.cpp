/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "lightflags.h"
#include "lightcomponent.h"

namespace nap
{
	uint getLightType(LightFlags flags)
	{
		return static_cast<uint>((flags >> 1U) & 0x7f);
	}

	uint getShadowSampleCount(LightFlags flags)
	{
		return static_cast<uint>((flags >> 8U) & 0xff);
	}

	bool isShadowSupported(LightFlags flags)
	{
		return getShadowSampleCount(flags) > 0U;
	}

	uint getShadowMapType(LightFlags flags)
	{
		return static_cast<uint>((flags >> 16U) & 0xff);
	}

	uint getLightIndex(LightFlags flags)
	{
		return static_cast<uint>((flags >> 24U) & 0xff);
	}

	LightFlags getLightFlags(const LightComponentInstance& light, uint index)
	{
		LightFlags flags = (static_cast<uint>(light.getLightType()) & 0xff);
		flags |= (static_cast<uint>(light.isShadowSupported() ? std::min(light.getShadowSampleCount(), 0xffU) : 0U) << 8U);
		flags |= (static_cast<uint>(light.getShadowMapType()) & 0xffU) << 16U;
		flags |= std::min(index, 0xffU) << 24U;

		return flags;
	}


	LightEnableFlags NAPAPI getLightEnableFlags(const LightComponentInstance& light)
	{
		LightEnableFlags flags = static_cast<uint>(light.isEnabled());
		flags |= light.isShadowSupported() ? static_cast<uint>(light.isShadowEnabled()) << 1 : 0;
		return flags;
	}
}
