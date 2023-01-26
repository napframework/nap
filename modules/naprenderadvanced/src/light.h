/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/resource.h>
#include <color.h>
#include <materialinstance.h>

namespace nap
{
	/**
	 * Base Light
	 */
	class NAPAPI Light : public Resource
	{
		RTTI_ENABLE(Resource)
	public:	
		/**
		 * Initialize
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	};


	/**
	 * Point
	 */
	class NAPAPI PointLight : public Light
	{
		RTTI_ENABLE(Light)
	public:
		/**
		 * Initialize
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		glm::vec3 mOrigin;
		RGBColorFloat mColor;
		float mIntensity;
	};


	/**
	 * Directional
	 */
	class NAPAPI DirectionalLight : public Light
	{
		RTTI_ENABLE(Light)
	public:
		/**
		 * Initialize
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		glm::vec3 mDirection;
		RGBColorFloat mColor;
		float mIntensity;
	};


	/**
	 * Spot
	 */
	class NAPAPI SpotLight : public Light
	{
		RTTI_ENABLE(Light)
	public:
		/**
		 * Initialize
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		glm::vec3 mOrigin;
		RGBColorFloat mColor;
		float mIntensity;

		float mInnerAngle;		// Light intensity is at maximum from center until inner angle of the cone
		float mOuterAngle;		// Light intensity falls off towards the outer angle of the cone
		float mRadius;			// The maximum radius of the cone
	};
}
