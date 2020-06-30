// Local Includes
#include "modulemanager.h"
#include "logger.h"
#include "service.h"
#include "module.h"
#include <nap/core.h>

// External Includes
#include <utility/fileutils.h>
#include <packaginginfo.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <fstream>

namespace nap
{
	ModuleManager::ModuleManager(nap::Core& core) :
		mCore(core)
	{
		initModules();
	}

	ModuleManager::~ModuleManager()
	{
		/*  Commented out for now because unloading modules can cause crashes in RTTR; during shutdown it will try
			to access RTTI types registered by potentially unloaded modules and crash because the pointers are no longer valid.
			
			This should probably be fixed in RTTR itself, but I'm not sure how yet. For now I've disabled the unloading of modules, 
			since this only happens during shutdown and modules will be unloaded then anyway.
		
		for (Module& module : mRequiredModules)
			UnloadModule(module.mHandle);
		*/
	}
	

}
