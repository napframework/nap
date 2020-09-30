/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include "texture2d.h"
#include "irendertarget.h"

namespace nap
{
	// Forward Declares
	class Core;

	/**
	 * Empty 2D GPU texture that can be declared as a resource. 
	 * It is often used to store the result of a render pass, for example by nap::RenderTarget.
	 */
	class NAPAPI RenderTexture2D : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		/**
		 * All supported render texture 2D formats.
		 */
		enum class EFormat
		{
			RGBA8,			///< 4 component(s), 1 byte per component
			R8				///< 1 component(s), 1 byte per component
		};

		RenderTexture2D(Core& renderService);

		/**
		 * Creates internal texture resource.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int					mWidth		= 0;								///< Property: 'Width' width of the texture in texels
		int					mHeight		= 0;								///< Property: 'Height' of the texture in texels
		EColorSpace			mColorSpace	= EColorSpace::Linear;				///< Property: 'ColorSpace' texture color space
		EFormat				mFormat		= EFormat::RGBA8;					///< Property: 'Format' texture format
	};
}
