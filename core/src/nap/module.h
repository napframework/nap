#pragma once

#include "component.h"
#include "logger.h"
#include "typeconverter.h"

namespace nap
{
	using TypeList = std::vector<RTTI::TypeInfo>;
    class ModuleManager;

	// The internal module keeps track of all necessary types that are to be exposed from within
	// this library.
	// Wraps the single internal nap::Core module, NOT to be used by client code directly.
	class Module
	{
        friend ModuleManager;
		RTTI_ENABLE()
	public:
		Module(const std::string& name);
		Module() {}
		virtual ~Module() = default;

		const std::string getName() const { return mName; }
		void setName(const std::string& name) { mName = name; }

		void getComponentTypes(TypeList& outTypes) const;
		void getDataTypes(TypeList& outTypes) const;
		void getOperatorTypes(TypeList& outTypes) const;
        void getServiceTypes(TypeList& outTypes);

		void setFilename(const std::string& filename) { mFilename = filename; }
        const std::string& getFilename() const { return mFilename; }

		const std::vector<const nap::TypeConverterBase*> getTypeConverters() const; // TODO: Make unique_ptr

		bool hasTypeConverter(const nap::TypeConverterBase* tc) const;

		void registerOperatorType(RTTI::TypeInfo type) { mOperatorTypes.push_back(type); }
		void registerComponentType(RTTI::TypeInfo type) { mComponentTypes.push_back(type); }
		void registerDataType(RTTI::TypeInfo type) { mDataTypes.push_back(type); }
        void registerServiceType(RTTI::TypeInfo type) { mServiceTypes.push_back(type); }

		template <typename I, typename O>
		void registerTypeConverter(bool (*func)(const I&, O&))
		{
			TypeConverter<I, O>* tc = new TypeConverter<I, O>(func);
			assert(!hasTypeConverter(tc));
			mTypeConverters.push_back(tc);
		}



	private:
        std::string mFilename;
		std::string mName;
		TypeList mOperatorTypes;
		TypeList mComponentTypes;
		TypeList mDataTypes;
        TypeList mServiceTypes;
		std::vector<const nap::TypeConverterBase*> mTypeConverters;
	};
}

RTTI_DECLARE_BASE(nap::Module)

//
// SHARED LIB EXPORT MACRO
//

// clang-format off
// Export macros
#ifdef _MSC_VER
#define NAP_EXPORT extern "C" __declspec(dllexport)
    #define NAP_IMPORT __declspec(dllimport)
#elif __GNUC__
#define NAP_EXPORT extern "C" __attribute__((visibility("default")))
#define NAP_IMPORT
#else //  do nothing and hope for the best?
#define NAP_EXPORT extern "C"
    #define NAP_IMPORT
    #pragma warning Unknown dynamic link import / export semantics.
#endif
// clang-format on


//
// MODULE UTILITIES
//

typedef nap::Module* (*init_module_fn)(void);

//#ifdef NAP_SHARED_LIBRARY

#define NAP_MODULE_BEGIN_DYN(NAME)            \
	NAP_EXPORT nap::Module* nap_init_module() \
	{                                         \
		auto module = new nap::Module(#NAME);

#define NAP_MODULE_END_DYN(NAME) \
	return module;               \
	}

#define NAP_REGISTER_OPERATOR_DYN(TYPE) module->registerOperatorType(RTTI_OF(TYPE));
#define NAP_REGISTER_DATATYPE_DYN(TYPE) module->registerDataType(RTTI_OF(TYPE));
#define NAP_REGISTER_COMPONENT_DYN(TYPE) module->registerComponentType(RTTI_OF(TYPE));
#define NAP_REGISTER_TYPECONVERTER_DYN(FUNC) module->registerTypeConverter(FUNC);

//#else // NAP_SHARED_LIBRARY

#define NAP_MODULE_BEGIN(NAME)                \
	class Module##NAME : public nap::Module   \
	{                                         \
		RTTI_ENABLE_DERIVED_FROM(nap::Module) \
	public:                                   \
	Module##NAME() : nap::Module(#NAME)

#ifdef NAP_SHARED_LIBRARY
#define NAP_MODULE_END(NAME)                  \
	}                                         \
	;                                         \
	RTTI_DECLARE(Module##NAME)                \
	RTTI_DEFINE(Module##NAME)                 \
	NAP_EXPORT nap::Module* nap_init_module() \
	{                                         \
		static Module##NAME m;                \
		return &m;                            \
	}
#else
#define NAP_MODULE_END(NAME)   \
	}                          \
	;                          \
	RTTI_DECLARE(Module##NAME) \
	RTTI_DEFINE(Module##NAME)
#endif

#define NAP_REGISTER_OPERATOR(TYPE) registerOperatorType(RTTI_OF(TYPE));
#define NAP_REGISTER_DATATYPE(TYPE) registerDataType(RTTI_OF(TYPE));
#define NAP_REGISTER_COMPONENT(TYPE) registerComponentType(RTTI_OF(TYPE));
#define NAP_REGISTER_SERVICE(TYPE) registerServiceType(RTTI_OF(TYPE));
#define NAP_REGISTER_TYPECONVERTER(FUNC) registerTypeConverter(FUNC);

//#endif // NAP_SHARED_LIBRARY