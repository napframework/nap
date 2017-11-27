#pragma once

#include <rtti/rtti.h>
#include <nap/signalslot.h>
#include <componentptr.h>
#include <nap/logger.h>
#include <utility/dllexport.h>
#include <pybind11/pybind11.h>

namespace nap
{
	class InputService;
	class PythonScriptComponent;

	class NAPAPI PythonScriptComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
        PythonScriptComponentInstance(EntityInstance& entity, Component& resource);

		virtual void update(double deltaTime) override;
		virtual bool init(utility::ErrorState& errorState) override;

        template <typename ...Args>
        void call(const std::string& identifier, Args... args);
        
	private:
		pybind11::module mScript;
        std::vector<ObjectPtr<ComponentInstance>> mComponentVariables;
	};


	/**
	 * The resource class for the PythonScriptComponent.
	 */
	class NAPAPI PythonScriptComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(PythonScriptComponent, PythonScriptComponentInstance)
        
	public:
		std::string mPath;
	};
    
    
    template <typename ...Args>
    void PythonScriptComponentInstance::call(const std::string& identifier, Args... args)
    {
        try {
            mScript.attr(identifier.c_str())(args...);
        }
        catch (const pybind11::error_already_set& err)
        {
            nap::Logger::info("Runtime python error while executing %s: %s", getComponent<PythonScriptComponent>()->mPath.c_str(), err.what());
        }
    }

}
