/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "lightflags.h"
#include "lightcomponent.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// LightFlags bits
	//////////////////////////////////////////////////////////////////////////

	enum LightFlagBits : uint
	{
		LIGHT_ENABLED_BITS = 1,
		SHADOW_ENABLED_BITS = 1,
		SHADOW_SUPPORTED_BITS = 1,
		LIGHT_TYPE_BITS = 0xff,
		SHADOW_MAP_TYPE_BITS = 0xff,
		LIGHT_INDEX_BITS = 0xff
	};

	enum LightFlagShifts : uint
	{
		LIGHT_ENABLED_SHIFT		= 0,
		SHADOW_ENABLED_SHIFT	= 1,
		SHADOW_SUPPORTED_SHIFT	= 2,
		LIGHT_TYPE_SHIFT		= 8,
		SHADOW_MAP_TYPE_SHIFT	= 16,
		LIGHT_INDEX_SHIFT		= 24
	};

	enum LightFlagMask : uint
	{
		LIGHT_ENABLED_MASK		= LIGHT_ENABLED_BITS << LIGHT_ENABLED_SHIFT,
		SHADOW_ENABLED_MASK		= SHADOW_ENABLED_BITS << SHADOW_ENABLED_SHIFT,
		SHADOW_SUPPORTED_MASK	= SHADOW_SUPPORTED_BITS << SHADOW_SUPPORTED_SHIFT,
		LIGHT_TYPE_MASK			= LIGHT_TYPE_BITS << LIGHT_TYPE_SHIFT,
		SHADOW_MAP_TYPE_MASK	= SHADOW_MAP_TYPE_BITS << SHADOW_MAP_TYPE_SHIFT,
		LIGHT_INDEX_MASK		= LIGHT_INDEX_BITS << LIGHT_INDEX_SHIFT
	};


	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Sets nth bit of value to x
	 */
	static void setBit(uint& outValue, uint n, bool x)
	{
		outValue = (outValue & ~(1U << n)) | (static_cast<uint>(x) << n);
	}


	//////////////////////////////////////////////////////////////////////////
	// LightFlags extraction
	//////////////////////////////////////////////////////////////////////////

	bool isLightEnabled(LightFlags flags)
	{
		return static_cast<uint>(flags & LIGHT_ENABLED_MASK) > 0;
	}

	bool isShadowEnabled(LightFlags flags)
	{
		return static_cast<uint>(flags & SHADOW_ENABLED_MASK) > 0;
	}

	bool isShadowSupported(LightFlags flags)
	{
		return static_cast<uint>(flags & SHADOW_SUPPORTED_MASK) > 0;
	}

	uint getLightType(LightFlags flags)
	{
		return static_cast<uint>(flags & LIGHT_TYPE_MASK) >> LIGHT_TYPE_SHIFT;
	}

	uint getShadowMapType(LightFlags flags)
	{
		return static_cast<uint>(flags & SHADOW_MAP_TYPE_MASK) >> SHADOW_MAP_TYPE_SHIFT;
	}

	uint getLightIndex(LightFlags flags)
	{
		return static_cast<uint>(flags & LIGHT_INDEX_MASK) >> LIGHT_INDEX_SHIFT;
	}

	LightFlags getLightFlags(const LightComponentInstance& light, uint index)
	{
		bool shadow_enabled = light.supportsShadows() ? light.isShadowEnabled() : false;

		LightFlags flags = 0;
		flags |= (static_cast<uint>(light.isEnabled()) & LIGHT_ENABLED_BITS)				<< LIGHT_ENABLED_SHIFT;
		flags |= (static_cast<uint>(shadow_enabled) & SHADOW_ENABLED_BITS)					<< SHADOW_ENABLED_SHIFT;
		flags |= (static_cast<uint>(light.supportsShadows()) & SHADOW_SUPPORTED_BITS)		<< SHADOW_SUPPORTED_SHIFT;
		flags |= (static_cast<uint>(light.getLightType()) & LIGHT_TYPE_BITS)				<< LIGHT_TYPE_SHIFT;
		flags |= (static_cast<uint>(light.getShadowMapType()) & SHADOW_MAP_TYPE_BITS)		<< SHADOW_MAP_TYPE_SHIFT;
		flags |= (index & LIGHT_INDEX_BITS)													<< LIGHT_INDEX_SHIFT;

		return flags;
	}


	ShadowFlags getShadowFlags(const std::vector<LightComponentInstance*> lights)
	{
		ShadowFlags flags = 0;
		for (uint i = 0; i < lights.size(); i++)
			setBit(flags, i, lights[i]->isShadowEnabled());

		return flags;
	}


	//////////////////////////////////////////////////////////////////////////
	// LightFlags update
	//////////////////////////////////////////////////////////////////////////

	void updateLightFlags(const LightComponentInstance& light, LightFlags& outFlags)
	{
		bool shadow_enabled = light.supportsShadows() ? light.isShadowEnabled() : false;
		setBit(outFlags, LIGHT_ENABLED_SHIFT, light.isEnabled());
		setBit(outFlags, SHADOW_ENABLED_SHIFT, shadow_enabled);
	}
}
