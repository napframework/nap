/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "rttiitem.h"

#include <QFileDialog>
#include <QStandardItem>
#include <rtti/typeinfo.h>
#include <nap/numeric.h>

namespace napkin
{
	namespace utility
	{
		/**
		 * Brings up a file picker
		 * @return path to a file to open
		 */
		QString getOpenFilename(QWidget *parent = nullptr, const QString& caption = {}, const QString& dir = {}, const QString& filter = {});

        /**
         * Brings up a file selector
         * @return path to file to save
         */
         QString getSaveFilename(QWidget* parent = nullptr, const QString& caption = {}, const QString& dir = {}, const QString& filter = {});

		/**
		 * NAP context and project root.
		 */
		class Context final
		{
		public:
			enum class EType : nap::int8
			{
				Source		= 0,	///< Napkin from NAP source
				Package		= 1,	///< Napkin from NAP binary package
				Application = 2,	///< Napkin from NAP binary application
				Unknown		= -1	///< Unresolved context
			};

			/**
			 * Context accessor.
			 *
			 * Figures out the NAP context by looking at the editor location within the project folder structure.
			 * This is useful to resolve paths before a project is loaded. The result is cached for faster lookup.
			 *
			 * @return the NAP context
			 */
			static const Context& get();

			/**
			 * @return the context Napkin is running in.
			 */
			EType getType() const							{ return mType; }

			/**
			 * @return the resolved NAP root, invalid (empty) when not known
			 */
			const QString& getRoot() const					{ return mRoot; }

		private:
			EType	mType = EType::Unknown;					///< Resolved context
			QString	mRoot = "";					            ///< NAP Root directory

			// Guesses the context when constructed
			Context();
		};
	}
}
