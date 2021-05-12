/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */


#pragma once

#include <nap/core.h>
#include <qobject.h>
#include <rtti/deserializeresult.h>
#include "document.h"

namespace napkin
{
	using ServiceConfigList = std::vector<std::unique_ptr<nap::rtti::Object>>;

	/**
	 * Provides functions to create, open and save a service configuration file.
	 * Initial configuration is copied from nap::Core on initialization.
	 */
	class ServiceConfig : public QObject
	{
		Q_OBJECT
	public:
		ServiceConfig(nap::Core& core);

		/**
		 * Returns file name associated with service configuration, can be null (not saved).
		 * If there's no filename the config is not associated with a file on disk.
		 * @return service config file name, can be null
		 */
		const QString& getFilename() const;

		/**
		 * Creates a new set of default service configuration objects,
		 * For every service configuration type associated with the active project.
		 */
		void newDefault();

		/**
		 * Loads service configuration settings from file.
		 * On startup service configuration settings are copied from Core, because
		 * Core already loads the configuration for us, performs additional checks,
		 * and creates default configurations if there is none specified for a specific service.
		 */
		void load(QString serviceConfigFile);

		/**
		 * Saves service configuration file to disk
		 */
		bool save();

		/**
		 * Saves service configuration to disk using given filename
		 */
		bool saveAs(const QString& fileName);

		/**
		 * Set current configuration as project default.
		 * For this operation to succeed the filename must be valid.
		 */
		bool makeDefault();

		/**
		 * Returns all current service configurations
		 */
		std::vector<nap::ServiceConfiguration*> getList() const;

		/**
		 * @return Document that manages configurations
		 */
		Document& getDocument()							{ return *mDocument; }

		/**
		 * @return Document that manages configurations
		 */
		const Document& getDocument() const				{ return *mDocument; }

	private:
		nap::Core& mCore;								// NAP Core reference
		QString mCurrentConfigFilename;					// Current configuration filename
		nap::rtti::OwnedObjectList mServiceConfigs;		// Current loaded service configuration
		std::unique_ptr<Document> mDocument = nullptr;	// Document that contains all the configurable objects

		/**
		 * Copies service configuration from core.
		 */
		void copyServiceConfig();
	};
}
