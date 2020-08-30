#pragma once

// Local Includes
#include "numeric.h"
#include "utility/dllexport.h"
#include "utility/errorstate.h"

// External Includes
#include <rtti/object.h>
#include <rtti/factory.h>
#include <set>

namespace nap
{
	// Forward Declares
	class Core;
	class ServiceObjectGraphItem;

	/**
	 * Base class for all Service configurations. In the derived class you can supply parameters that can be used to initialize a service.
	 */
	class NAPAPI ServiceConfiguration : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
	public:
		/**
		 *	@return The Service type associated with this configuration.
		 */
		virtual rtti::TypeInfo getServiceType() = 0;
	};


	/**
	 * A Service is a process within core that cooperates with certain components in the system, this is the base
	 * class for all services. Often services are used to load a driver, set up a connection or manage global module
	 * specific state. All services are automatically loaded and managed by Core. This ensures the right service order of
	 * initialization, runtime state and closing. When designing a module using a service make sure to export the
	 * service using the 'NAP_SERVICE_MODULE' #define in a source file exactly once, for example:
	 *
	 *~~~~~{.cpp}
	 * NAP_SERVICE_MODULE("mod_naposc", "0.2.0", "nap::OSCService")
	 *~~~~~
	 *
	 * This code snippet ensures that core automatically loads, creates and initializes the service when loading the modules.
	 **/
	class NAPAPI Service
	{
		RTTI_ENABLE()
		friend class Core;
		friend class ServiceObjectGraphItem;
	public:
		Service(ServiceConfiguration* configuration);

		// Virtual destructor because of virtual methods!
		virtual ~Service();

		/**
		 *	@return the nap core this service belongs to
		 */
		Core& getCore();

		/**
		 *	@return the type name of the service
		 */
		const std::string getTypeName() const;

		/**
		 * Copy is not allowed
		 */
		Service(Service&) = delete;

		/**
		 * Copy assignment is not allowed
		 */
		Service& operator=(const Service&) = delete;

		/**
		 * Move is not allowed
		 */
		Service(Service&&) = delete;

		/**
		 * Move assignment is not allowed
		 */
		Service& operator=(Service&&) = delete;

	protected:
		/**
		 * Override this function to register specific object creators for classes associated with this module
		 * @param factory the factory used by the resource manager to instantiate objects
		 */
		virtual void registerObjectCreators(rtti::Factory& factory)						{ }

		/**
		 * Override this function to register service dependencies
		 * A service that depends on another service is initialized after all it's associated dependencies
		 * This will ensure correct order of initialization, update calls and shutdown of all services
		 * @param dependencies rtti information of the services this service depends on
		 */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies)			{ }

		/**
		 * Invoked when the service has been constructed and Core is available.
		 * This occurs before service initialization
		 */
		virtual void created()															{}

		/**
		 * Invoked by core after initializing the core engine. 
		 * When called all modules this service depends on have been initialized
		 * Override this method to initialize your service.
		 * @param error should contain the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& error)									{ return true; }

		/**
		 * Invoked by core in the app loop. Update order depends on service dependency.
		 * This call is invoked before the resource manager checks for file changes and the app update call
		 * If service B depends on A, A::update() is called before B::update()
		 * @param deltaTime: the time in seconds between calls
		 */
		virtual void preUpdate(double deltaTime)										{ }

		/**
		 * Invoked by core in the app loop. Update order depends on service dependency
		 * This call is invoked after the resource manager has loaded any file changes but before
		 * the app update call. If service B depends on A, A:s:update() is called before B::update()
		 * @param deltaTime: the time in seconds between calls
		 */
		virtual void update(double deltaTime)											{ }

		/**
		 * Invoked by core in the app loop. Update order depends on service dependency
		 * This call is invoked after the application update call
		 * @param deltaTime: the time in seconds between calls
		 */
		virtual void postUpdate(double deltaTime)										{ }

		/**
		 * Invoked when exiting the main loop, after app shutdown is called
		 * This is called before shutdown() and before the resources are destroyed.
		 * Use this function if your service needs to reset its state before resources 
		 * are destroyed
		 * When service B depends on A, Service B is shutdown before A
		 */
		virtual void preShutdown()														{ }

		/**
		 * Invoked when exiting the main loop, after app shutdown is called
		 * Use this function to close service specific handles, drivers or devices
		 * When service B depends on A, Service B is shutdown before A
		 */
		virtual void shutdown()															{ }

		/**
		 * Invoked when the resource manager is about to load resources
		 */
		virtual void preResourcesLoaded()												{ }

		/**
		 * Invoked after the resource manager successfully loaded resources
		 */
		virtual void postResourcesLoaded()												{ }

		/**
		 * Retrieve the ServiceConfiguration for this service. Will be null if this service does not support configuration
		 * @return The ServiceConfiguration
		 */
		template<typename SERVICE_CONFIG>
		SERVICE_CONFIG* getConfiguration()
		{
			return rtti_cast<SERVICE_CONFIG>(mConfiguration);
		}

		/**
		* Retrieve the ServiceConfiguration for this service. Will be null if this service does not support configuration
		* @return The ServiceConfiguration
		*/
		template<typename SERVICE_CONFIG>
		const SERVICE_CONFIG* getConfiguration() const
		{
			return rtti_cast<SERVICE_CONFIG>(mConfiguration);
		}

	private:
		// this variable will be set by the core when the service is added
		Core*									mCore = nullptr;
		ServiceConfiguration*					mConfiguration;
	};
}
