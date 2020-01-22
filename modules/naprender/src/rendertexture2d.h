#pragma once

// External Includes
#include "texture2d.h"

namespace nap
{
	/**
	 * GPU texture resource that it is initially empty.
	 * This texture can be declared as a resource together with
	 * the format to use, width and height.
	 */
	class NAPAPI RenderTexture2D : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
	public:
		enum class EFormat : int
		{
			RGBA8 = 4,			///< RGBA8 4 components, 8 bytes per component
			RGB8  = 3,			///< RGB8 3 components, 8 bytes per component
			R8	  = 1,			///< R8	1 components, 8 bytes per component
			Depth = 0			///< Depth Texture used for binding to depth buffer
		};

		/**
		 * @return number of channels associated with this render texture
		 */
		int getChannelCount();

		/**
		 * Creates internal texture resource.
		 * @param errorState Contains error state if the function fails.
		 * @return if the texture was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		int		mWidth  = 0;					///< Property: 'Width' width of the texture in texels
		int		mHeight = 0;					///< Property: 'Height' of the texture, in texels
		EFormat	mFormat = EFormat::RGB8;		///< Property: 'Format' format of the texture
	};
}
