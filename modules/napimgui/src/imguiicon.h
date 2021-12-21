/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "imgui/imgui.h"

// External Includes
#include <rtti/rtti.h>
#include <nap/resource.h>
#include <imagefromfile.h>

namespace nap
{
	// Forward Declares
	class Core;
	class IMGuiService;

	/**
	 * Generic icon that can be displayed in a GUI.
	 */
	class NAPAPI Icon : public Resource
	{
		RTTI_ENABLE()
	public:
		/**
		 * Creates the icon. 
		 * @param guiService gui reference
		 */
		Icon(nap::IMGuiService& guiService);

		/**
		 * Creates the icon.
		 * @param guiService gui reference
		 * @param imagePath path, absolute or relative, to icon
		 */
		Icon(nap::IMGuiService& guiService, const std::string& imagePath);

		/**
		 * Loads the icon and creates icon LODs.
		 * Note that until this function is called, the Icon will not have a valid name.
		 * @param error contains the error if loading fails
		 * @return if loading succeeded.
		 */
		virtual bool init(utility::ErrorState& error);

		/**
		 * @return path to icon.
		 */
		const std::string& getPath() const					{ return mImagePath; }

		/**
		 * @return name of the icon, without extension;
		 */
		const std::string& getName() const					{ return mName; }

		/**
		 * @return 2DTexture that can be rendered
		 */
		const nap::Texture2D& getTexture() const			{ return mImage; }

		/**
		 * @return the GUI Service
		 */
		IMGuiService& getGuiService()						{ return mGuiService; }

		/**
		 * Returns the GUI texture handle of this icon.
		 * @return the GUI texture handle
		 */
		ImTextureID getTextureHandle() const;

		std::string mImagePath;								///< Property: 'ImagePath' path to the image on disk

	private:
		nap::ImageFromFile mImage;							//< Texture to display
		IMGuiService& mGuiService;							//< GUI Service
		std::string mName;									//< Icon name, without extension
	};

	using IconObjectCreator = rtti::ObjectCreator<Icon, IMGuiService>;
}
