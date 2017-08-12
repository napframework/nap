#include <pybind11/pybind11.h>
#include <nap/logger.h>
#include <iostream>
#include <nap/core.h>


int add(int i, int j) {
    return i + j;
}

int subtract(int i, int j) {
    return i - j;
}

nap::Core &core() {
    static nap::Core c;
    c.initialize();
    return c;
}

namespace py = pybind11;

PYBIND11_MODULE(nap, m) {

    core();
    nap::rtti::PythonModule &python_module = nap::rtti::PythonModule::get("nap");
    python_module.invoke(m);

    m.def("initialize", &core);

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
