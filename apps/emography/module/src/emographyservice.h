#pragma once

// External Includes
#include <nap/service.h>
#include <entity.h>

namespace nap
{
	/**
	 * Service associated with emography module.
	 * Manages global emography state.
	 */
	class NAPAPI EmographyService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default Constructor
		EmographyService(ServiceConfiguration* configuration);

		/**
		 * @return database storage directory, different for various platforms
		 */
		std::string getDBSourceDir() const 				{ return mDatabaseSourceDir; }

		/**
		 * Sets the database storage dir, only necessary for android	
		 */
		void setDBSourceDir(const std::string& dir)		{ mDatabaseSourceDir = dir; }

	protected:
		/**
		 * Register specific object creators, used when creating the resources from file.
		 * In this case we want to populate the data-model with this service, 
		 * in order to acquire the right database path later on.
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * Initializes the etherdream library.
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 *	Shutdown the etherdream library
		 */
		virtual void shutdown() override;

	private:
		// Source directory of the database file
		std::string mDatabaseSourceDir = "";
	};
}