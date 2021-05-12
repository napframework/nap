#pragma once

#include <nap/core.h>
#include <qobject.h>

namespace napkin
{
	/**
	 * Napkin Service Configuration Model
	 * Provides utility functions to create, open and save a service configuration file.
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
		void newServiceConfig();

		/**
		 * Loads service configuration settings from file.
		 * On startup service configuration settings are copied from Core, because
		 * Core already loads the configuration for us, performs additional checks,
		 * and creates default configurations if there is none specified for a specific service.
		 */
		void loadServiceConfig(QString serviceConfigFile);

		/**
		 * Saves service configuration file to disk
		 */
		bool saveServiceConfig();

		/**
		 * Saves service configuration to disk using given filename
		 */
		bool saveServiceConfigAs(const QString& fileName);

		/**
		 * Set current configuration as project default.
		 * For this operation to succeed the filename must be valid.
		 */
		bool setAsDefault();

	private:
		nap::Core& mCore;															// NAP Core reference
		QString mCurrentConfigFilename;												// Current configuration filename
		std::vector<std::unique_ptr<nap::ServiceConfiguration>> mServiceConfigs;	// Current loaded service configuration

		/**
		 * Copies service configuration from core.
		 */
		void copyServiceConfig();
	};
}
