/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "texture.h"

// External Includes
#include <rtti/factory.h>

namespace nap
{
	/**
	 * Checkerboard test texture to inspect UV coordinates.
	 */
	class NAPAPI UVTestTexture : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		/**
		 * @param core the core instance.
		 */
		UVTestTexture(Core& core);

		/**
		 * Loads the uv grid texture from disk and schedules the upload to the GPU on success.
		 * @param errorState contains the error when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	public:
		bool mGenerateLods = true;					///< Property: 'GenerateLods' If LODs are generated for this image
	};
}

