#pragma once

#include "scriptinterpreter.h"
#include <Python.h>

namespace nap
{
	class PythonInterpreter : public ScriptInterpreter
	{
		RTTI_ENABLE_DERIVED_FROM(ScriptInterpreter)
	public:
		PythonInterpreter() : ScriptInterpreter() { Py_Initialize(); }

		std::string evalScript(const std::string& cmd) override
		{
            // Redirect stdout
            std::stringbuf ss;
            std::streambuf* oldStream = std::cout.rdbuf(&ss);

			PyCompilerFlags* flags = nullptr;

			PyObject* m = PyImport_AddModule("__main__");
			if (!m)
				return "ERROR";

			PyObject* d = PyModule_GetDict(m);
			PyObject* v = PyRun_StringFlags(cmd.c_str(), Py_file_input, d, d, flags);
			if (!v) {
				PyErr_Print();
				return "";
			}

            PyObject* vRep = PyObject_Repr(v);
            std::string ret = PyString_AsString(vRep);

			Py_DECREF(v);
			if (Py_FlushLine())
				PyErr_Clear();

			return ret;
		}
	};
}

RTTI_DECLARE(nap::PythonInterpreter)