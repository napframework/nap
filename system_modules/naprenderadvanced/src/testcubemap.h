/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

// Local includes
#include "equirectangularcubemap.h"

namespace nap
{
	/**
	 * A basic labeled 6-plane cubemap designed for testing purposes.
	 */
	class NAPAPI TestCubeMap : public EquiRectangularCubeMap
	{
		RTTI_ENABLE(EquiRectangularCubeMap)
	public:
		// Constructor
		TestCubeMap(Core& core);

		/**
		 * Generate the test cubemap.
		 * @param errorState the error when initialization fails
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		Texture2D mEquiRectangularTexture;
	};


	/**
	 * A basic 6-plane sunset cubemap designed for testing purposes
	 */
	class NAPAPI SunsetCubeMap : public EquiRectangularCubeMap
	{
		RTTI_ENABLE(EquiRectangularCubeMap)
	public:
		// Constructor
		SunsetCubeMap(Core& core);

		/**
		 * Generate the sunset cubemap.
		 * @param errorState contains the error message when initialization fails
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		Texture2D mEquiRectangularTexture;
	};
}

