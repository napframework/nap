#pragma once

#include "utility/dllexport.h"

namespace nap
{
	/**
	 * Struct used to describe a particular module to the system. Contains the API version that the module was built against, which is used for forwards/backwards compatibility.
	 * New members can be added to this struct, as long as the API version is updated and they are added *after* the APIVersion member
	 */
	struct ModuleDescriptor
	{
		static const int	ModuleAPIVersion = 1; // Current version of the module API version

		int					mAPIVersion;	// The version of the module API this module was built against. Must be the first member.
		const char*			mID;			// The ID (name) of the module
		const char*			mVersion;		// The version of the module
		const char*			mService;		// The service associated with the module
	};

	/**
	 * Macro used to define (and on windows, export) the descriptor for a particular module. Should appear exactly once in the source for a module, in a cpp.
	 */
	#define NAP_MODULE(moduleID, moduleVersion)											\
		extern "C"																		\
		{																				\
			NAPAPI nap::ModuleDescriptor descriptor =									\
			{																			\
				nap::ModuleDescriptor::ModuleAPIVersion,								\
				moduleID,																\
				moduleVersion,															\
				nullptr																	\
			};																			\
		}

	/**
	 * Macro used to define (and on windows, export) the descriptor for a particular module with a service. Should appear exactly once in the source for a module, in a cpp.
	 */
	#define NAP_SERVICE_MODULE(moduleID, moduleVersion, moduleService)						\
			extern "C"																		\
			{																				\
				NAPAPI nap::ModuleDescriptor descriptor =									\
				{																			\
					nap::ModuleDescriptor::ModuleAPIVersion,								\
					moduleID,																\
					moduleVersion,															\
					moduleService															\
				};																			\
			}
}