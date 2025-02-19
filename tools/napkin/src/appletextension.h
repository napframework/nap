/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "naputils.h"

// External includes
#include <nap/coreextension.h>
#include <nap/projectinfo.h>
#include <assert.h>

namespace napkin
{
	class AppletRunner;

	/**
	 * Napkin applet extension for NAP core.
	 * Exposes the project being edited in Napkin to all available applets.
	 */
	class AppletExtension final : public nap::CoreExtension
	{
		friend class AppletRunner;
		RTTI_ENABLE(nap::CoreExtension)
	public:
		/**
		 * Creates the applet core extension
		 * @param project the project being edited in Napkin
		 */
		AppletExtension(std::unique_ptr<nap::ProjectInfo> project) :
			mProjectInfo(std::move(project)) { }

		/**
		 * Creates the applet core extension without referencing a project
		 */
		AppletExtension() = default;

		/**
		 * Changes the current working directory to the data directory that is being edited in Napkin.
		 * Allows data to be de-serialized relative to the napkin project directory, instead of the applet.
		 * The current working directory is active until the handle falls out of scope.
		 * @return Napkin working directory handle
		 */
		CWDHandle switchWorkingDir() const			{ assert(mProjectInfo != nullptr); return CWDHandle(mProjectInfo->getDataDirectory()); }

		/**
		 * @return if project is referenced
		 */
		bool hasProject() const						{ return mProjectInfo != nullptr; }

		/**
		 * Returns project being edited in Napkin, nullptr if not available 
		 * @return project being edited in Napkin
		 */
		const nap::ProjectInfo* getProject() const	{ return mProjectInfo.get(); }

	private:
		std::unique_ptr<nap::ProjectInfo> mProjectInfo = nullptr;
	};
}

