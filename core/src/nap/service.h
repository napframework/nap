#pragma once

// Local Includes
#include "configure.h"
#include "utility/dllexport.h"

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>
#include <set>

namespace nap
{
	// Forward Declares
	class Core;

	/**
	 @brief Service
	 A Service is a process within the core that cooperates with certain components in the system, this is the base
	 class for all services
	 **/

	class NAPAPI Service
	{
		RTTI_ENABLE()
		friend class Core;
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

		/**
		 *	Invoked when the service has been constructed and Core is available.
		 */
		virtual void initialized() {}

	protected:
		/**
		 * Override this function to register specific object creators for classes associated with this module
		 * @param factory the factory used by the resource manager to instantiate objects
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) {}

	private:
		// this variable will be set by the core when the service is added
		Core* mCore = nullptr;
	};
}
