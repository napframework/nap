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

		// Returns the nap Core this service belongs to
		Core& getCore();

		// Service type name
		const std::string getTypeName() const;;

		// Invoked when the service has been constructed and Core is available.
		virtual void initialized() {}

	protected:
		// Registers all available object creation functions associated with a module
		virtual void registerObjectCreators(rtti::Factory& factory) {}

	private:
		// this variable will be set by the core when the service is added
		Core* mCore = nullptr;
	};
}
