#pragma once

#include <functional>
#include <vector>
#include "utility/dllexport.h"

#include "pybind11/pybind11.h"
#include "pybind11/embed.h"
#include "rttr/detail/misc/misc_type_traits.h"

#define _STRINGIFY(X) #X
#define STRINGIFY(X) _STRINGIFY(X)

#define PYBIND11_EMBEDDED_MODULE_WRAPPER(NAME) PYBIND11_EMBEDDED_MODULE(NAME, module)

#define REGISTER_MODULE_PYTHON_BINDINGS() PYBIND11_EMBEDDED_MODULE_WRAPPER(MODULE_NAME)					\
{																										\
	nap::rtti::PythonModule& python_module = nap::rtti::PythonModule::get(STRINGIFY(MODULE_NAME));		\
	python_module.invoke(module);																		\
}

namespace nap
{
	namespace rtti
	{
		namespace detail
		{
			template<typename T>
			inline void GetTypeNamesImpl(std::vector<std::string>& result)
			{
				result.push_back(rtti::TypeInfo::get<T>().get_name().data());
			}

			template<typename... Targs>
			inline void GetTypeNames(std::vector<std::string>& result);

			template<typename T, typename... V>
			inline void GetTypeNamesHelper(std::vector<std::string>& result)
			{
				GetTypeNamesImpl<T>(result);
				GetTypeNames<V...>(result);
			}

			template<typename... Targs>
			inline void GetTypeNames(std::vector<std::string>& result)
			{
				GetTypeNamesHelper<Targs...>(result);
			}

			template<>
			inline void GetTypeNames<>(std::vector<std::string>& result)
			{
			}
		}	

		template<class T, class BaseClasses>
		class PythonClass;

		template<class T, typename... BaseClasses>
		class PythonClass<T, rttr::detail::type_list<BaseClasses...>>
		{
		public:
			using PybindClass = pybind11::class_<T, BaseClasses...>;
			using RegistrationFunction = std::function<void(PybindClass&)>;

			PythonClass(const std::string& name)
			{
				int namespace_separator_pos = name.find_last_of(':');
				if (namespace_separator_pos != std::string::npos)
					mName = name.substr(namespace_separator_pos+1);
				else
					mName = name;
			}

			void registerFunction(const RegistrationFunction& function)
			{
				mRegistrationFunctions.push_back(function);
			}

			void invoke(pybind11::module& module) const
			{
				auto cls = PybindClass(module, mName.c_str());
				for (auto& func : mRegistrationFunctions)
					func(cls);
			}

			static void GetBaseTypes(std::vector<std::string>& dependentTypes)
			{
				detail::GetTypeNames<BaseClasses...>(dependentTypes);
			}

		private:
			std::string mName;
			std::vector<RegistrationFunction> mRegistrationFunctions;
			std::vector<std::string> mBaseTypes;
		};

		class NAPAPI PythonModule
		{
		public:
			using RegistrationFunction = std::function<void(pybind11::module&)>;
			using GetBaseTypesFunction = std::function<void(std::vector<std::string>&)>;

			static PythonModule& get(const char* moduleName);

			void registerImportCallback(RegistrationFunction function);
			void registerTypeImportCallback(const std::string& name, GetBaseTypesFunction getBaseTypesFunction, RegistrationFunction registrationFunction);
			void invoke(pybind11::module& module);

		private:
			struct RegistrationItem
			{
				GetBaseTypesFunction		mGetBaseTypesFunction;
				RegistrationFunction		mRegistrationFunction;
			};
			using RegistrationMap = std::unordered_map<std::string, RegistrationItem>;

			void registerTypeRecursive(pybind11::module& module, RegistrationMap::iterator itemToRegister);
			
			RegistrationMap mRegistrationMap;
			std::vector<RegistrationFunction> mImportCallbacks;
		};
	}
}