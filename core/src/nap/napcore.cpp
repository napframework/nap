#include "python.h"
#include "utility/module.h"

#ifdef NAP_ENABLE_PYTHON
PYBIND11_EMBEDDED_MODULE(nap, module)
{
	nap::rtti::PythonModule& python_module = nap::rtti::PythonModule::get("nap");
	python_module.invoke(module);
}
#endif // NAP_ENABLE_PYTHON

NAP_MODULE("napcore", "0.2.0")

