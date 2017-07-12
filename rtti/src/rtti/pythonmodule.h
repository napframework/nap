#pragma once

#include <functional>
#include <vector>

#include "pybind11/pybind11.h"
#include "pybind11/embed.h"

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
		template<class T>
		class PythonClass
		{
		public:
			using RegistrationFunction = std::function<void(pybind11::class_<T>&, pybind11::module&)>;

			PythonClass(const char* name) :
				mName(name)
			{
			}

			void registerFunction(const RegistrationFunction& function)
			{
				mRegistrationFunctions.push_back(function);
			}

			void invoke(pybind11::module& module) const
			{
				auto cls = pybind11::class_<T>(module, mName);
				for (auto& func : mRegistrationFunctions)
					func(cls, module);
			}

		private:
			const char* mName;
			std::vector<RegistrationFunction> mRegistrationFunctions;
		};

		class PythonModule
		{
		public:
			using RegistrationFunction = std::function<void(pybind11::module&)>;

			static PythonModule& get(const char* moduleName);

			void registerClass(RegistrationFunction function);

			void invoke(pybind11::module& module);

		private:
			std::vector<RegistrationFunction> mClassRegistrations;
		};
	}
}