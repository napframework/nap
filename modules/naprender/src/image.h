#pragma once

#include <texture2d.h>
#include <bitmap.h>

namespace nap
{
	/**
	 * Represents both CPU and GPU data for a texture. The CPU data is stored internally as a bitmap and is optional.
	 * Classes should derive from Texture2D and call either initTexture (for a GPU-only texture) or initFromBitmap (for a CPU & GPU texture).
	 * GPU textures can be read back to CPU using the getData functions. This will fill the internal bitmap with the data read-back from the GPU.
	 * A texture can be modified in two ways:
	 * - By modifying the internal bitmap (retrieved through getBitmap()) and calling update(). This is the most common way of updating the texture.
	 *   When updating the texture in this way, the formats & size of the CPU and GPU textures are guaranteed to match.
	 * - By calling update directly with a custom data buffer. This is useful if you already have data available and don't want the extra overhead of 
	 *   copying to the internal bitmap first. When updating the texture in this way, you are responsible for making sure that the data buffer you pass in 
	 *   matches the format & size of the GPU texture.
	 */
	class NAPAPI Image : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
			
	public:
		using Texture2D::update;
		using Texture2D::getData;
		using Texture2D::endGetData;

		/**
		 *	@return CPU data for this texture in the form of a Bitmap. The Bitmap can be empty if this is a GPU-only texture.
		 */
		Bitmap& getBitmap() { return mBitmap; }

		/**
		 *	Converts the CPU data in the internal Bitmap to the GPU. The bitmap should contain valid data and not be empty.
		 */
		void update();

		/**
		 * Blocking call to retrieve GPU texture data that is stored in this texture
		 * When the internal bitmap is empty it will be initialized based on the settings associated with this texture
		 * This call asserts if the bitmap can't be initialized or, when initialized, the bitmap settings don't match.
		 * @return reference to the internal bitmap that is filled with the GPU data of this texture.
		 */
		Bitmap& getData();

		/**
		 * Starts a transfer of texture data from GPU to CPU. This is a non blocking call.
		 * For performance, it is important to start a transfer as soon as possible after the texture is rendered.
		 */
		void startGetData();

		/**
		* Finishes a transfer of texture data from GPU to CPU that was started with startGetData. See comment in startGetData for proper use.
		* When the internal bitmap is empty it will be initialized based on the settings associated with this texture.
		* This call asserts if the bitmap can't be initialized or, when initialized, the bitmap settings don't match.
		* @return reference to the internal bitmap that is filled with the GPU data of this texture.
		*/
		Bitmap& endGetData();

	private:
		Bitmap		mBitmap;			///< The CPU image representation
	};
}

