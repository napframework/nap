#include <pybind11/pybind11.h>
#include <nap/core.h>

int add(int i, int j) {
    return i + j;
}

int subtract(int i, int j) {
    return i - j;
}

namespace py = pybind11;

PYBIND11_PLUGIN(pynap) {
    py::module m("pynap");

    m.def("add", &add);

    m.def("subtract", &subtract);

    py::class_<nap::Core>(m, 'Core');

#ifdef VERSION_INFO
    m.attr("__version__") = py::str(VERSION_INFO);
#else
    m.attr("__version__") = py::str("dev");
#endif

    return m.ptr();
}
