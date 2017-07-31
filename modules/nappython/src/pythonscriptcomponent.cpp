#include <pythonscriptcomponent.h>
#include <nap/entity.h>
#include "nap/fileutils.h"

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::PythonScriptComponentInstance, nap::EntityInstance&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PythonScriptComponent)
	RTTI_PROPERTY("Path", &nap::PythonScriptComponent::mPath, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

namespace nap
{
	pybind11::module PythonScriptComponentInstance::mScript;

	void PythonScriptComponentInstance::update(double deltaTime)
	{
		try
		{
			mScript.attr("update")(deltaTime);
		}
		catch (const pybind11::error_already_set& err)
		{
			int i = 0; 
		}
	}

	bool PythonScriptComponentInstance::init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		const PythonScriptComponent* scriptComponent = rtti_cast<PythonScriptComponent>(resource.get());
		
		try 
		{
			PyObject* sysPath = PySys_GetObject((char*)"path");
			PyList_Append(sysPath, Py_BuildValue("s", getAbsolutePath(getFileDir(scriptComponent->mPath)).c_str()));

			if (!mScript.ptr())
			{
				mScript = pybind11::module::import(getFileNameWithoutExtension(scriptComponent->mPath).c_str());
			}
			else
			{
				mScript = pybind11::reinterpret_steal<pybind11::module>(PyImport_ReloadModule(mScript.ptr()));
			}

			if (!errorState.check(mScript.ptr(), "Failed to reload %s; check for syntax errors", scriptComponent->mPath.c_str()))
				return false;
		}
		catch (const pybind11::error_already_set& err)
		{
			errorState.fail(err.what());
			return false;
		}		 
		// TODO: check script

		return true;
	}
}