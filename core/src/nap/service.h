#pragma once

// Local Includes
#include "configure.h"
#include "utility/dllexport.h"
#include "utility/errorstate.h"

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>
#include <set>

namespace nap
{
	// Forward Declares
	class Core;
	class ServiceObjectGraphItem;

	/**
	 @brief Service
	 A Service is a process within the core that cooperates with certain components in the system, this is the base
	 class for all services
	 **/

	class NAPAPI Service
	{
		RTTI_ENABLE()
		friend class Core;
		friend class ServiceObjectGraphItem;
	public:
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

	protected:
		/**
		 * Override this function to register specific object creators for classes associated with this module
		 * @param factory the factory used by the resource manager to instantiate objects
		 */
		virtual void registerObjectCreators(rtti::Factory& factory)					{}

		/**
		 * Override this function to register service dependencies
		 * A service that depends on another service is initialized after all it's associated dependencies
		 * This will ensure correct order of initialization and update calls
		 * @param dependencies rtti information of the services this service depends on
		 */
		virtual void getDependencies(std::vector<rtti::TypeInfo>& dependencies)		{}

		/**
		 * Invoked when the service has been constructed and Core is available.
		 */
		virtual void created() {}

		/**
		 * Invoked by core after creation. When called all modules this service depends on have been initialized
		 * Override this method to initialize your service.
		 * @param error
		 */
		virtual bool init(utility::ErrorState& error)								{ return true; }

	private:
		// this variable will be set by the core when the service is added
		Core* mCore = nullptr;
	};
}
