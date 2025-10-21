/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "texturepreviewapicomponent.h"

// External includes
#include <utility/errorstate.h>
#include <rtti/typeinfo.h>

namespace napkin
{
	class TexturePreviewApplet;
	using namespace nap;

	/**
	 * Creates and draws the GUI of the texture preview applet
	 */
	class TexturePreviewAppletGUI final
	{
	public:
		/**
		 * Gui requires full access to applet
		 */
		TexturePreviewAppletGUI(TexturePreviewApplet& applet) :
			mApplet(applet)												{ }

		/**
		 * Create and update the gui for this frame
		 * @param elapsedTime time elapsed between frames in seconds
		 */
		void update(double elapsedTime);

		/**
		 * Draw the gui
		 */
		void draw();

	private:
		void texDetail(std::string&& label, const std::string& value, std::string&& appendix = "");
		void texDetail(std::string&& label, rtti::TypeInfo enumerator, rtti::Variant argument);
		void updateTexture2D(TexturePreviewAPIComponentInstance& controller);
		void updateTextureCube(TexturePreviewAPIComponentInstance& controller);
		TexturePreviewApplet& mApplet;
	};
}


