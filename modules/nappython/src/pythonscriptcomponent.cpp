#include <pythonscriptcomponent.h>
#include <nap/entity.h>
#include "nap/fileutils.h"
#include "nap/core.h"
#include "pythonscriptservice.h"
#include "nap/logger.h"

RTTI_BEGIN_CLASS_CONSTRUCTOR1(nap::PythonScriptComponentInstance, nap::EntityInstance&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PythonScriptComponent)
	RTTI_PROPERTY("Path", &nap::PythonScriptComponent::mPath, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

namespace nap
{
	void PythonScriptComponentInstance::update(double deltaTime)
	{
		try
		{
			mScript.attr("update")(deltaTime);
		}
		catch (const pybind11::error_already_set& err)
		{
			nap::Logger::info("Runtime python error while executing %s: %s", mScriptComponent->mPath.c_str(), err.what());
		}
	}

	bool PythonScriptComponentInstance::init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		mScriptComponent = rtti_cast<PythonScriptComponent>(resource.get());

		PythonScriptService* script_service = getEntity()->getCore()->getOrCreateService<PythonScriptService>();
		if (!errorState.check(script_service->TryLoad(mScriptComponent->mPath, mScript, errorState), "Failed to load %s", mScriptComponent->mPath.c_str()))
			return false;
		
		return true;
	}
}