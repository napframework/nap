#pragma once

#include <rtti/rtti.h>
#include <nap/signalslot.h>
#include "component.h"
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
		PythonScriptComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
		{
		}

		virtual void update(double deltaTime) override;
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		pybind11::module mScript;
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
}