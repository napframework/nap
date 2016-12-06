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

	// Responsible for loading, keeping and querying modules.
	class ModuleManager
	{
	public:
		ModuleManager();


		void loadModules(const std::string directory = "modules");
		bool hasModule(const Module& module);

		const TypeConverterBase* getTypeConverter(RTTI::TypeInfo fromType,
												  RTTI::TypeInfo toType) const;

		//        const Proxy* getComponentProxy(const std::string& name) const;

		const TypeList getComponentTypes() const;

		const TypeList getDataTypes() const;

		const TypeList getOperatorTypes() const;

		const std::vector<Module*>& getModules() const { return mModules; }

        Module* getModule(const std::string& name) const;

		Signal<Module&> moduleLoaded;

	private:
		void registerRTTIModules();

		void registerModule(Module& mod);
		std::vector<Module*> mModules;

		TypeConverterPassThrough mTypeConverterPassThrough;
	};
}
