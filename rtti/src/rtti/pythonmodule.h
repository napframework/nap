/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <functional>
#include <vector>
#include <utility/dllexport.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

// Note: including rttr headers directly here instead of going through typeinfo.h to avoid circular dependencies between the two headers
#include <rttr/detail/misc/misc_type_traits.h>
#include <rttr/type>

namespace nap
{
	namespace rtti
	{
		namespace detail
		{
			/** 
			 * GetTypeNames is a helper to retrieve a runtime list of type names out of variadic template args. The types of 
			 * the arguments need to be registered into RTTI. When calling this function before types are registered, they 
			 * will receive a temporary name that may not match the (custom) name that is passed when the value is actually
			 * registered. So be sure to call this function at the moment when all types are registered.
			 *
			 * GetTypeNames is used to extract the BaseClasses in string format that was extracted from RTTR (see comments in typeinfo.h).
			 * The reason why we need to extract these is because pybind unfortunately requires an explicit initialization 
			 * order: base classes need to registered first, then derived classes. By extracting them to a vector of strings
			 * we can put the type names in maps and vectors and make sure that when performing the registration of all types, we can do 
			 * so in the correct order.
			 */
			template<typename T>
			inline void GetTypeNamesImpl(std::vector<std::string>& result)
			{
				result.push_back(rttr::type::get<T>().get_name().data());
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

		/**
		 * Represents a single class registered for python. Each instance holds a list of registration functions. These functions register
		 * properties and functions into pybind11. The RTTI macro's are responsible for creating an instance of PythonClass and calling
		 * registerFunction for each property and function.
		 *
		 * PythonClasses are gathered into PythonModules. When a PythonModule is imported, PythonClass:invoke is called, which will call
		 * the registration functions for all properties and functions. These registration functions should register the necessary
		 * data into the pybind module and/or python class.
		 * 
		 * Here is a simplified example how the RTTR macros expand to python registration functions:
		 * 
		 *		nap::rtti::PythonClass<args> python_class;															<= RTTI_BEGIN_CLASS,		Defines a python class
		 *		python_class.registerFunction([](pybind11::module& module, PythonClassType::PybindClass& cls)		<= RTTI_PROPERTY (1)		Register Foo::SomeProperty callback, this'll be called when module is imported
		 *		{																									<= RTTI_PROPERTY (1)
		 *			cls.def_readwrite("SomeProperty", &Foo::SomeProperty);											<= RTTI_PROPERTY (1)
		 *		});																									<= RTTI_PROPERTY (1)
		 *		python_class.registerFunction([](pybind11::module& module, PythonClassType::PybindClass& cls)		<= RTTI_PROPERTY (2)		Register Bar::SomeProperty callback, this'll be called when module is imported
		 *		{																									<= RTTI_PROPERTY (2)
		 *			cls.def_readwrite("SomeProperty", &Bar::SomeProperty);											<= RTTI_PROPERTY (2)
		 *		});																									<= RTTI_PROPERTY (2)
		 * 		nap::rtti::PythonModule& python_module = nap::rtti::PythonModule::get("nap");						<= RTTI_END_CLASS			Retrieve the "nap" module and register this class. See notes at PythonModule why we use a single module
	 	 *		python_module.registerTypeImportCallback(rtti_class_type_name,										<= RTTI_END_CLASS
		 *											 [](std::vector<std::string>& baseTypes)						<= RTTI_END_CLASS			Register getBaseTypes callback for this type into this module
		 *											 {																<= RTTI_END_CLASS
		 *												PythonClassType::GetBaseTypes(baseTypes);					<= RTTI_END_CLASS
		 *											 },																<= RTTI_END_CLASS
		 *											 [python_class](py::module& module)								<= RTTI_END_CLASS			Register class invoke into module
		 *											 {																<= RTTI_END_CLASS
		 *												python_class.invoke(module);								<= RTTI_END_CLASS
		 *											 });															<= RTTI_END_CLASS
		 */
		template<class T, typename... BaseClasses>
		class PythonClass<T, rttr::detail::type_list<BaseClasses...>>
		{
		public:
			using PybindClass = pybind11::class_<T, BaseClasses...>;
			using RegistrationFunction = std::function<void(pybind11::module&, PybindClass&)>;

			/**
			 * Constructor
			 * @param name Name of the class.
			 */
			PythonClass(const std::string& name)
			{
				// Strip namespace out of type name
				int namespace_separator_pos = name.find_last_of(':');
				if (namespace_separator_pos != std::string::npos)
					mName = name.substr(namespace_separator_pos+1);
				else
					mName = name;
			}

			/**
			 * Registers a callback function into the class.
			 */
			void registerFunction(const RegistrationFunction& function)
			{
				mRegistrationFunctions.push_back(function);
			}

			/**
			 * Calls class registration functions for properties and functions
			 */
			void invoke(pybind11::module& module) const
			{
				auto cls = PybindClass(module, mName.c_str());
				for (auto& func : mRegistrationFunctions)
					func(module, cls);
			}

			/**
			 * Returns list of base class type names for this class.
			 * Notice that the RTTI types need to be registered, otherwise a temporary name may be 
			 * returned that may not match the final type name exactly.
			 */
			static void GetBaseTypes(std::vector<std::string>& baseClasses)
			{
				detail::GetTypeNames<BaseClasses...>(baseClasses);
			}

		private:
			std::string mName;												///< Name of this class
			std::vector<std::string> mBaseTypes;							///< Names of base classes
			std::vector<RegistrationFunction> mRegistrationFunctions;		///< Callbacks for registering functions and properties
		};

		/**
		 * Represents a single python module. In python, when loading a .py file, it will contain import statements. These import statements import 'modules'.
		 * These modules need to be registered in pybind11 to interface with C++. 
		 * 
		 * Modules are registered through PYBIND11_EMBEDDED_MODULE. For example, the module named "nap" is defined like this:
		 *
		 *		PYBIND11_EMBEDDED_MODULE(nap, module)
		 *		{
		 *				nap::rtti::PythonModule& python_module = nap::rtti::PythonModule::get("nap");
		 *				python_module.invoke(module);
		 *		}
		 * Whenever an import statement is encountered in python, this code will be called. As you can see, the invoke function for the nap module is called here.
		 * 
		 * <<IMPORTANT>>
		 * We chose to have all our code inside a single "nap" module instead of multiple modules (for instance, per dll). The reason is that automatic upcasting
		 * (an important feature) can only be done for inheritance hierarchies within the same module. It is possible to support this across modules, but we went
		 * for the easier route, as it would mean even more complexity than we already have.
		 * That said, it is possible to create multiple modules manually if you find a good use for it. Currently, the RTTI macro's use "nap" as the hardcoded
		 * module name. This could be parametrized, or registration functions could be inserted into custom modules manually through registerImportCallback or
		 * registerTypeImportCallback.
		 */
		class NAPAPI PythonModule
		{
		public:
			using RegistrationFunction = std::function<void(pybind11::module&)>;
			using GetBaseTypesFunction = std::function<void(std::vector<std::string>&)>;

			/**
			 * Returns the global python module for the given module name.
			 * @param moduleName name of the module to get
			 * @return the python module
			 */
			static PythonModule& get(const char* moduleName);

			/**
			 * Register a function that is called when the module is imported.
			 */
			void registerImportCallback(RegistrationFunction function);

			/**
			 * Register class registration functions that are called when the module is imported.
			 * @param name The class name
			 * @param getBaseTypesFunction	The function that returns the base classes for a class as a list of strings.
			 * @param registrationFunction  The function responsible for registering the property or method into pybind11.
			 */
			void registerTypeImportCallback(const std::string& name, GetBaseTypesFunction getBaseTypesFunction, RegistrationFunction registrationFunction);
			
			/**
			 * Called when the module is imported.
			 */
			void invoke(pybind11::module& module);

		private:
			/**
			 * Class registration item containing information about base classes and registration functions.
			 */
			struct RegistrationItem
			{
				GetBaseTypesFunction		mGetBaseTypesFunction;
				RegistrationFunction		mRegistrationFunction;
			};
			using RegistrationMap = std::unordered_map<std::string, RegistrationItem>;

			/**
			 * When types are registered, we go through our base classes first to make sure that these are registered first.
			 * Therefore we use a recursive function recurse into the base classes. After an item is registered, we remove
			 * it from the map so that it isn't registered multiple times.
			 */
			void registerTypeRecursive(pybind11::module& module, RegistrationMap::iterator itemToRegister);
			
			RegistrationMap mRegistrationMap;						///< Map from class name to Registration Item
			std::vector<RegistrationFunction> mImportCallbacks;		///< Regular callbacks when module is imported.
		};
	}
}
