/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "texture.h"
#include "irendertarget.h"

namespace nap
{
	/**
	 * Simple texture 2D layout synchronization interface.
	 * Allows external code to update texture layout after rendering operations.
	 * This is a low-level operation and is therefore concealed from the general interface.
	 */
	class NAPAPI Texture2DLink
	{
	public:
		// Destructor
		virtual ~Texture2DLink() = default;

		// Copy is not allowed
		Texture2DLink(Texture2DLink&) = delete;
		Texture2DLink& operator=(const Texture2DLink&) = delete;

		// Move is not allowed
		Texture2DLink(Texture2DLink&&) = delete;
		Texture2DLink& operator=(Texture2DLink&&) = delete;

	protected:
		// Only derived classes can create it
		Texture2DLink() = default;

		/**
		 * Manual texture 2D layout synchronization.
		 * Updates texture 2D layout to match the specified target layout
		 * @param tex texture to sync
		 * @param new texture image layout
		 */
		static void sync(Texture2D& tex, VkImageLayout layout) { tex.mImageData.mCurrentLayout = layout; }
	};


	/**
	 * Texture 2D layout synchronization interface for 2D render targets.
	 * Allows render targets to update the texture layout after completion of a render pass.
	 * This is a low-level operation and is therefore concealed from the general interface.
	 */
	class NAPAPI Texture2DTargetLink : public Texture2DLink
	{
	public:
		// Constructor
		Texture2DTargetLink(const IRenderTarget& target) : mTarget(target) {};

		/**
		 * Synchronizes texture layout with final layout of the render target
		 * @param tex the texture to synchronize
		 */
		void sync(Texture2D& tex) { Texture2DLink::sync(tex, mTarget.getFinalLayout()); }

	private:
		const IRenderTarget& mTarget;
	};


	/**
	 * Simple texture cube layout synchronization interface.
	 * Allows external objects to modify texture layout after rendering operations.
	 * This is a low-level operation and is therefore concealed from the general interface.
	 */
	class NAPAPI TextureCubeLink
	{
	public:
		// Destructor
		virtual ~TextureCubeLink() = default;

		// Copy is not allowed
		TextureCubeLink(TextureCubeLink&) = delete;
		TextureCubeLink& operator=(const TextureCubeLink&) = delete;

		// Move is not allowed
		TextureCubeLink(TextureCubeLink&&) = delete;
		TextureCubeLink& operator=(TextureCubeLink&&) = delete;

	protected:
		// Only derived classes can create it
		TextureCubeLink() = default;

		/**
		 * Manual texture cube layout synchronization.
		 * Updates texture cube layout to match the specified target layout.
		 * @param tex texture to sync
		 * @param new texture image layout
		 */
		static void sync(TextureCube& tex, VkImageLayout layout) { tex.mImageData.mCurrentLayout = layout; }
	};


	/**
	 * Texture cube layout synchronization interface for cube render targets.
	 * Allows a cube render target to update the texture layout after completion of a render pass.
	 * This is a low-level operation and is therefore concealed from the general interface.
	 */
	class NAPAPI TextureCubeTargetLink : public TextureCubeLink
	{
	public:
		TextureCubeTargetLink(const IRenderTarget& target) : mTarget(target) {};

		/**
		 * Synchronizes texture layout with final layout of the render target
		 * @param tex the texture to synchronize
		 */
		void sync(TextureCube& tex) { TextureCubeLink::sync(tex, mTarget.getFinalLayout()); }

	private:
		const IRenderTarget& mTarget;
	};
}

