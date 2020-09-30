/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <texture2d.h>
#include <bitmap.h>

namespace nap
{
	/**
	 * Represents both CPU and GPU data of a texture. The CPU data is stored internally as a bitmap and is optional.
	 * GPU textures can be read back to CPU using the asyncGetData() function. This will fill the internal bitmap with the data read-back from the GPU.
	 * To update the GPU texture, update the bitmap (using getBitmap()) and call update() afterwards.
	 * The format & size of the CPU and GPU textures are guaranteed to match afterwards.
	 */
	class NAPAPI Image : public Texture2D
	{
		RTTI_ENABLE(Texture2D)
			
	public:
		using Texture2D::update;
		using Texture2D::asyncGetData;

		/**
		 * @param core the core instance.
		 */
		Image(Core& core);

		/**
		 * @return CPU data for this texture in the form of a Bitmap. The Bitmap can be empty if this is a GPU-only texture.
		 */
		Bitmap& getBitmap() { return mBitmap; }

		/**
		 * Uploads the CPU data in the internal Bitmap to the GPU. The bitmap should contain valid data and not be empty.
		 */
		void update();

		/**
		 * Starts a transfer of texture data from GPU to CPU. This is a non blocking call.
		 * For performance, it is important to start a transfer as soon as possible after the texture is rendered.
		 */
		void asyncGetData();

	private:
		Bitmap		mBitmap;			///< The CPU image representation
	};
}

