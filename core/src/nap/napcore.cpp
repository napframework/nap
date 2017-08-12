#include "rtti/pythonmodule.h"

// The following statically initializes a python interpreter on library access.
// It prevents a python plugin module to be imported because there may only be one interpreter
// This should probably be done in a generic executable (core_app.exe?) instead
// At the least, the end-user of NAP should probably decide whether this is enabled or not.

//PYBIND11_EMBEDDED_MODULE(nap, module)
//{
//	nap::rtti::PythonModule& python_module = nap::rtti::PythonModule::get("nap");
//	python_module.invoke(module);
//}
