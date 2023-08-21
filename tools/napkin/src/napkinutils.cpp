/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "napkinutils.h"
#include "napkinglobals.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/core.h>
#include <shader.h>

namespace napkin
{
	namespace utility
	{
		QString getOpenFilename(QWidget* parent, const QString& caption, const QString& dir, const QString& filter)
		{
#ifdef __linux__
			return QFileDialog::getOpenFileName(parent, caption, dir, filter, nullptr, QFileDialog::DontUseNativeDialog);
#else
			return QFileDialog::getOpenFileName(parent, caption, dir, filter, nullptr);
#endif
		}


		/**
		 * Guesses the Napkin context by looking at it's location within the folder structure.
		 */
		Context::Context()
		{
			QDir root_dir = QString::fromStdString(nap::utility::getExecutableDir());
			assert(root_dir.exists());

			// Package Release
			root_dir.cdUp();
			if (root_dir.dirName().toLower() == DIR_TOOLS)
			{
				root_dir.cdUp();
				mRoot = root_dir.path();
				mType = Context::EType::Package;
				return;
			}

			// Application Release
			if (QFileInfo(root_dir.path(), PROJECT_INFO_FILENAME).exists())
			{
				mType = Context::EType::Application;
				mRoot = root_dir.path();
				return;
			}

			// Source
			root_dir.cdUp();
			if (root_dir.dirName().toLower() == DIR_BIN)
			{
				root_dir.cdUp();
				mRoot = root_dir.path();
				mType = Context::EType::Source;
			}
		}
		
		const napkin::utility::Context& Context::get()
		{
			static Context sContext;
			return sContext;
		}
	}
}
