#pragma once

// Local
#include "module.h"
#include "component.h"
#include "fileutils.h"
#include "logger.h"
#include "attribute.h"
#include "signalslot.h"

// External
#include <functional>

namespace nap
{
	/**
	 * Responsible for loading, keeping and querying modules.
	 * The Module system is heavily in flux and will likely change a lot
	 * over time, so, forgive for not commenting
	 */

	class ModuleManager
	{
	public:
		ModuleManager();

		/**
		 * TODO: This should probably be private and can only be called from Core
		 * Initializes the NAP module that exposes and registers indispensable base types
		 */
        void loadCoreModule();

		/**
		 * Attempts to dynamically load all modules (plugins) from the specified directory
		 * @param directory the directory that holds all modules, this directory is local to the working directory
		 */
		void loadModules(const std::string directory = ".");

		/**
		 * Tries to find the correct type converter based on @fromtype and @toType
		 * @return a converter, nullptr if not found
		 */
		const TypeConverterBase* getTypeConverter(rtti::TypeInfo fromType, rtti::TypeInfo toType) const;

		const TypeList getComponentTypes() const;

		const TypeList getDataTypes() const;

		const TypeList getOperatorTypes() const;

		const std::vector<Module*>& getModules() const { return mModules; }

        Module* getModule(const std::string& name) const;

		Signal<Module&> moduleLoaded;
        void registerRTTIModules();

	private:
	   /**
		* checks if the module is already loaded, based on the module's name
		*/
		bool hasModule(const Module& module);

		/**
		 * Start managing module
		 * @param mod: the module to add and keep track of
		 */
		void registerModule(Module& mod);

		// All registered modules
		std::vector<Module*> mModules;

		// If source and target type are the same this
		// type converter will be used, does nothing
		TypeConverterPassThrough mTypeConverterPassThrough;
	};
}
