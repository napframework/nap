/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include <utility/dllexport.h>
#include <nap/numeric.h>

namespace nap
{
	// Forward declares
	class LightComponentInstance;

	// Persistent light information coded in a uint32
	// (msb)[index : 8bit][map id : 8bit][shadow samples : 8bit][type : 8bit](lsb)
	using LightFlags = uint;

	// Dynamic light enable information coded in a uint32
	// (msb)[padding : 30bit][shadow : 1bit][light : 1bit](lsb)
	using LightEnableFlags = uint;

	/**
	 * @return the light type
	 */
	uint NAPAPI getLightType(LightFlags flags);

	/**
	 * @return the shadow sample count
	 */
	uint NAPAPI getShadowSampleCount(LightFlags flags);

	/**
	 * @return whether shadow is supported
	 */
	bool NAPAPI isShadowSupported(LightFlags flags);

	/**
	 * @return the shadow map type
	 */
	uint NAPAPI getShadowMapType(LightFlags flags);

	/**
	 * @return the light index
	 */
	uint NAPAPI getLightIndex(LightFlags flags);

	/**
	 * @return the light flags of the specified light
	 */
	LightFlags NAPAPI getLightFlags(const LightComponentInstance& light, uint index);

	/**
	 * @return the light enable flags of the specified light
	 */
	LightEnableFlags NAPAPI getLightEnableFlags(const LightComponentInstance& light);
}
