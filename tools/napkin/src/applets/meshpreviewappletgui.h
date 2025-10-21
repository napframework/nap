/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <utility/errorstate.h>
#include <rtti/typeinfo.h>
#include <glm/vec3.hpp>

namespace napkin
{
	class MeshPreviewApplet;
	using namespace nap;

	/**
	 * Creates and draws the GUI of the mesh preview applet
	 */
	class MeshPreviewAppletGUI final
	{
	public:
		/**
		 * Gui requires full access to applet
		 */
		MeshPreviewAppletGUI(MeshPreviewApplet& applet);

		/**
		 * Initialize gui based on current applet state
		 */
		void init();

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
		MeshPreviewApplet& mApplet;
	};
}


