/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "dllexport.h"

// External includes
#include <string>

/**
 * Every NAP module exports a descriptor (symbol) that is loaded by the system (editor & app) on initialization.
 * It is important that you expose your nap module using either the 'NAP_MODULE' or 'NAP_SERVICE_MODULE' macro, failure to
 * do so will result in the system being unable to inspect and load the module on initialization.
 *
 * Call 'NAP_MODULE' to register a module without a service.
 * Call 'NAP_SERVICE_MODULE' to register a module with a service.
 *
 * The macro must be defined in a .cpp file **exactly once** for every module, for example:
 *
 * ~~~~~{.cpp}
 * #include <utility/module.h>
 * NAP_SERVICE_MODULE("napinput", "0.2.0", "nap::InputService")
 * ~~~~~
 *
 * The module descriptor is also made available to every class that is (RTTI) defined and can be accessed
 * using 'nap::rtti::getModuleDescription()', this allows the editor to determine - for example -
 * the origin of a class.
 *
 * If the target being compiled is not a nap module the NAP_MODULE_DESCIPTOR_HANDLE is null.
 * Otherwise the NAP_MODULE_DESCIPTOR_HANDLE expands to the location of the descriptor in the target module.
 */
namespace nap
{
	constexpr int moduleAPIVersion = 1;						///< Current  module API version
	constexpr const char* moduleCoreName = "napcore";		///< Core module name id

	/**
	 * Struct used to describe a particular module to the system. Contains the API version that the module was built against, which is used for forwards/backwards compatibility.
	 * New members can be added to this struct, as long as the API version is updated and they are added *after* the APIVersion member
	 */
	struct ModuleDescriptor
	{
		int			mAPIVersion;	// The version of the module API this module was built against. Must be the first member.
		const char* mID;			// The ID (name) of the module
		const char* mVersion;		// The version of the module
		const char* mService;		// The service associated with the module
	};

	/**
	 * Returns the shared library module descriptor symbol name
	 * @return module descriptor symbol name
	 */
	inline std::string getModuleDescriptorSymbolName(const std::string& moduleName) { return moduleName + "_mod_descriptor"; }
}


//////////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////////

#ifdef NAP_SHARED_LIBRARY
	// Module descriptor helper macros
	#define NAP_MODULE_CONCAT(x,y) x ## y
	#define NAP_MODULE_UNIQUE_NAME(x,y)  NAP_MODULE_CONCAT(x,y)
	#define NAP_MODULE_ADD_NS(x) nap::x

	// Module descriptor symbol name without namespace
	#define NAP_MODULE_SYMBOL_NAME NAP_MODULE_UNIQUE_NAME(MODULE_NAME, _mod_descriptor)

	// Module descriptor handle (pointer) with namespace
	#define NAP_MODULE_DESCIPTOR_HANDLE &NAP_MODULE_ADD_NS(NAP_MODULE_SYMBOL_NAME)
#else
	#define NAP_MODULE_DESCIPTOR_HANDLE nullptr
#endif // NAP_SHARED_LIBRARY

namespace nap
{
	/**
	 * Macro used to define (and on windows, export) the descriptor for a particular module.
	 * Should appear exactly once in the source for a module, in a cpp.
	 */
	#define NAP_MODULE(moduleID, moduleVersion)											\
		extern "C"																		\
		{																				\
			NAPAPI nap::ModuleDescriptor NAP_MODULE_SYMBOL_NAME =						\
			{																			\
				nap::moduleAPIVersion,								                    \
				moduleID,																\
				moduleVersion,															\
				nullptr																	\
			};																			\
		}

	/**
	 * Macro used to define (and on windows, export) the descriptor for a particular module with a service.
	 * Should appear exactly once in the source for a module, in a cpp.
	 */
	#define NAP_SERVICE_MODULE(moduleID, moduleVersion, moduleService)						\
			extern "C"																		\
			{																				\
				NAPAPI nap::ModuleDescriptor NAP_MODULE_SYMBOL_NAME =						\
				{																			\
					nap::moduleAPIVersion,								                    \
					moduleID,																\
					moduleVersion,															\
					moduleService															\
				};																			\
			}

	/**
	 * Module descriptor declaration.
	 * Must be defined exactly once per module in a .cpp file using the NAP_MODULE or NAP_SERVICE_MODULE macros above.
	 */
	extern "C" 
	{
	#ifdef NAP_MODULE_SYMBOL_NAME
		extern nap::ModuleDescriptor NAP_MODULE_SYMBOL_NAME;
	#endif // NAP_MODULE_DECLARATION
	}
}
