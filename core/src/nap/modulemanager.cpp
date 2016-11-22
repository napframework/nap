#include <nap/coreattributes.h>
#include <nap/coremodule.h>
#include <nap/modulemanager.h>

// clang-format off

#ifdef _WIN32
	#include <windows.h> // Windows dll loading
#else
	#include <dlfcn.h> // Posix shared object loading
#endif

#ifdef _WIN32
	std::string GetLastErrorAsString()
	{
		//Get the error message, if any.
		DWORD errorMessageID = ::GetLastError();
		if(errorMessageID == 0)
			return std::string(); //No error message has been recorded

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;
	}
#endif


namespace nap
{

	Module* loadModule(const char* filename)
	{
		std::stringstream errorString;

		// First load library and get a pointer

		#ifdef _WIN32
			#ifdef _MSC_VER
				void* libPtr = LoadLibrary(filename);
			#else
				void* libPtr = LoadLibrary((LPCSTR)filename);
			#endif

			if (!libPtr) {
				Logger::warn("Shared library load failed: %s, reason: %s", filename, GetLastErrorAsString().c_str());
				return nullptr;
			}

		#else
			void* libPtr = dlopen(filename, RTLD_LAZY);

			if (!libPtr) {
				Logger::warn("Shared library load failed: %s, %s", filename, dlerror());
				return nullptr;
			}
		#endif



		// Load the initialization function

		const char* fn_name = "nap_init_module";

		#ifdef _WIN32
			init_module_fn init_module = (init_module_fn)GetProcAddress((HINSTANCE)libPtr, fn_name);
		#else
				init_module_fn init_module = (init_module_fn)dlsym(libPtr, fn_name);
		//		char* error = dlerror();
		#endif

		if (!init_module) {
			Logger::debug("Failed to load init function: %s", fn_name);
			return nullptr;
		}



		// Initialize the plugin

		Module* module = init_module();
		if (nullptr == module) {
			Logger::warn("Failed to initialize the plugin");
			return nullptr;
		}

		module->setFilename(filename);

		return module;
	}



	bool ModuleManager::hasModule(const Module& module)
	{
		for (const auto& mod : mModules) {
			std::string modNameA = module.getName();
			std::string modNameB = mod->getName();
			if (modNameB == modNameA)
				return true;
		}
		return false;
	}

	void ModuleManager::loadModules(const std::string directory)
	{
		std::string fullPath(getAbsolutePath(directory));
		Logger::debug("Looking for modules in directory: '%s'", fullPath.c_str());
		std::vector<std::string> files;
		nap::listDir(directory.c_str(), files);


		for (const auto& filename : files) {
			std::string absFilename = getAbsolutePath(filename);
#ifdef _WIN32
			if (getFileExtension(absFilename) != "dll")
				continue;
#endif

			//			Logger::debug("Attempting to load module '%s'", getAbsolutePath(filename).c_str());

			Module* module = loadModule(absFilename.c_str());
			if (!module) {
				Logger::warn("Failed to load module '%s'", absFilename.c_str());
				continue;
			}

			registerModule(*module);
		}
	}


	void ModuleManager::registerModule(Module& module)
	{
		if (hasModule(module)) {
			Logger::warn("Module already exists: %s", module.getName().c_str());
			return;
		}
		mModules.push_back(&module);
		moduleLoaded.trigger(module);
		Logger::info("Module registered: %s", module.getName().c_str());
	}

	ModuleManager::ModuleManager()
	{
		registerModule(*new ModuleNapCore());
		registerRTTIModules();
	}

	const TypeConverterBase* ModuleManager::getTypeConverter(RTTI::TypeInfo fromType, RTTI::TypeInfo toType) const
	{
		if (fromType == toType)
			return &mTypeConverterPassThrough;

		for (auto module : mModules) {
			for (auto conv : module->getTypeConverters()) {
				if (conv->inType() == fromType && conv->outType() == toType)
					return conv;
			}
		}
		return nullptr;
	}

	const TypeList ModuleManager::getComponentTypes() const
	{
		TypeList types;
		for (const auto& type : RTTI::TypeInfo::getRawTypes()) {
			if (type.isKindOf<Component>() && type.canCreateInstance())
				types.push_back(type);
		}
		// TODO: Bring this version back to life (needs proper module/type registration)
		//		for (const auto& mod : getModules())
		//			mod->getComponentTypes(types);
		return types;
	}

	const TypeList ModuleManager::getDataTypes() const
	{
		TypeList types;
		for (const auto& mod : getModules())
			mod->getDataTypes(types);
		return types;
	}

	const TypeList ModuleManager::getOperatorTypes() const
	{
		TypeList types;
		for (const auto& mod : getModules())
			mod->getOperatorTypes(types);
		return types;
	}

	void ModuleManager::registerRTTIModules()
	{
		for (RTTI::TypeInfo& typeInfo : RTTI::TypeInfo::getRawTypes()) {

			if (!typeInfo.isKindOf<nap::Module>())
				continue;
			if (!typeInfo.canCreateInstance())
				continue;

			nap::Module* module = (nap::Module*)typeInfo.createInstance();
			assert(module);
			nap::Logger::info("Module RTTI: %s", module->getName().c_str());
			registerModule(*module);
		}
	}

	Module* ModuleManager::getModule(const std::string& name) const
	{
		for (Module* module : mModules)
			if (module->getName() == name)
				return module;
		return nullptr;
	}
}


// clang-format on