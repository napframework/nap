/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include <utility/dllexport.h>
#include <nap/numeric.h>
#include <vector>

namespace nap
{
	// Forward declares
	class LightComponentInstance;

	/**
	 * Persistent light information coded in a uint32
	 * (msb)[index : 8bit][map id : 8bit][type : 8bit][padding : 5bit][shadow supported : 1bit][shadow enabled : 1bit][light enabled : 1bit](lsb)
	 */
	using LightFlags = uint;

	/**
	 * @return whether the light is marked as enabled
	 */
	bool NAPAPI isLightEnabled(LightFlags flags);

	/**
	 * @return whether shadows are marked as enabled
	 */
	bool NAPAPI isShadowEnabled(LightFlags flags);

	/**
	 * @return whether shadows are marked as supported
	 */
	bool NAPAPI isShadowSupported(LightFlags flags);

	/**
	 * @return the light type
	 */
	uint NAPAPI getLightType(LightFlags flags);

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
	 * 
	 */
	void NAPAPI updateLightFlags(const LightComponentInstance& light, LightFlags& outFlags);


	/**
	 * Persistent shadow toggle information coded in a uint32
	 * (index:32)[shadow enabled per light : 32bit](index:0)
	 */
	using ShadowFlags = uint;

	/**
	 *
	 */
	ShadowFlags NAPAPI getShadowFlags(const std::vector<LightComponentInstance*> lights);
}
