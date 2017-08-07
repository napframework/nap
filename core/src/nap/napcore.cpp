#include "rtti/pythonmodule.h"

PYBIND11_EMBEDDED_MODULE(nap, module)
{
	nap::rtti::PythonModule& python_module = nap::rtti::PythonModule::get("nap");
	python_module.invoke(module);
}
