#include <pybind11/pybind11.h>
#include <nap/logger.h>
#include <iostream>
#include <nap/core.h>



nap::Core &core() {
    static bool initialized = false;
    static nap::Core c;
    if (!initialized) {
        c.initialize();
        initialized = true;
    }
    return c;
}

namespace py = pybind11;

PYBIND11_MODULE(nap, m) {

    core();
    nap::rtti::PythonModule &python_module = nap::rtti::PythonModule::get("nap");
    python_module.invoke(m);


//    m.attr("core") = &core();
//    m.attr("one") = 10;
//
//    py::class_<nap::Core>(m, "Core")
//            .def("getElapsedTime", &nap::Core::getElapsedTime);

#ifdef VERSION_INFO
    m.attr("__version__") = py::str(VERSION_INFO);
#else
    m.attr("__version__") = py::str("dev");
#endif

}

int main(int argc, char** argv) {
    std::cout << "Hullo..." << std::endl;
    return 0;
}
