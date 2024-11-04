/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "dllexport.h"

// External includes
#include <string>

#ifdef MODULE_NAME
	#define NAP_MODULE_PASTER(x,y) x ## y
	#define NAP_MODULE_EVALUATOR(x,y)  NAP_MODULE_PASTER(x,y)
	#define NAP_MODULE_DECLARATION NAP_MODULE_EVALUATOR(MODULE_NAME, _descriptor)
#endif // MODULE_NAME

namespace nap
{
    constexpr int moduleAPIVersion = 1;						///< Current  module API version
	constexpr const char* coreModuleName = "napcore";		///< Core module name id

	/**
	 * Returns the shared library module descriptor symbol name
	 * @return module descriptor symbol name
	 */
	inline std::string getModuleSymbolName(const std::string& moduleName) { return moduleName + "_descriptor"; }

	/**
	 * Struct used to describe a particular module to the system. Contains the API version that the module was built against, which is used for forwards/backwards compatibility.
	 * New members can be added to this struct, as long as the API version is updated and they are added *after* the APIVersion member
	 */
	struct ModuleDescriptor
	{
		int					mAPIVersion;	// The version of the module API this module was built against. Must be the first member.
		const char*			mID;			// The ID (name) of the module
		const char*			mVersion;		// The version of the module
		const char*			mService;		// The service associated with the module
	};

	/**
	 * Module descriptor declaration.
	 * Must be defined exactly once per module in a .cpp file using the NAP_MODULE or NAP_SERVICE_MODULE macro below.
	 */
	extern "C" 
	{
#ifdef NAP_MODULE_DECLARATION
		extern nap::ModuleDescriptor NAP_MODULE_SYMBOL_NAME;
#endif // NAP_MODULE_DECLARATION
	}

	/**
	 * Macro used to define (and on windows, export) the descriptor for a particular module.
	 * Should appear exactly once in the source for a module, in a cpp.
	 */
	#define NAP_MODULE(moduleID, moduleVersion)											\
		extern "C"																		\
		{																				\
			NAPAPI nap::ModuleDescriptor NAP_MODULE_DECLARATION =						\
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
				NAPAPI nap::ModuleDescriptor NAP_MODULE_DECLARATION =						\
				{																			\
					nap::moduleAPIVersion,								                    \
					moduleID,																\
					moduleVersion,															\
					moduleService															\
				};																			\
			}

	/**
	 * Returns module description handle if available.
	 * The handle is only available when building a nap library, not an executable.
	 * @return module description handle if available.
	 */
	inline const nap::ModuleDescriptor* getDescriptor()
	{
#ifdef NAP_MODULE_SYMBOL_NAME
		return &NAP_MODULE_DECLARATION;
#else
		return nullptr;
#endif
	}
}
