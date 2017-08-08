#include "rtti/pythonmodule.h"
#include "utility/module.h"

PYBIND11_EMBEDDED_MODULE(nap, module)
{
	nap::rtti::PythonModule& python_module = nap::rtti::PythonModule::get("nap");
	python_module.invoke(module);
}

NAP_MODULE("napcore", "1.0")

