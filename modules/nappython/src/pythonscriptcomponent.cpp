/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "pythonscriptcomponent.h"
#include "pythonscriptservice.h"

// External includes
#include <entity.h>
#include <utility/fileutils.h>
#include <nap/core.h>
#include <nap/logger.h>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PythonScriptComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PythonScriptComponent)
    RTTI_PROPERTY("PythonScript", &nap::PythonScriptComponent::mPythonScript, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Class", &nap::PythonScriptComponent::mClassName, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Dependencies", &nap::PythonScriptComponent::mDependencies, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    
    PythonScriptComponentInstance::PythonScriptComponentInstance(EntityInstance& entity, Component& resource) :
        ComponentInstance(entity, resource)
    {
        mResource = rtti_cast<PythonScriptComponent>(&resource);
    }

    
    PythonScriptComponentInstance::~PythonScriptComponentInstance()
    {
        if (mInitialized)
        {
            utility::ErrorState errorState;
            if (!call("destroy", errorState))
                nap::Logger::warn(errorState.toString());
        }
    }
    
    
    void PythonScriptComponentInstance::update(double deltaTime)
    {
        utility::ErrorState errorState;
        if (!call("update", errorState, getEntityInstance()->getCore()->getElapsedTime(), deltaTime))
            nap::Logger::warn(errorState.toString());
    }

    
    bool PythonScriptComponentInstance::init(utility::ErrorState& errorState)
    {
        mResource = getComponent<PythonScriptComponent>();
        
        mInstance = mResource->mPythonClass(getEntityInstance());
        
        return true;
    }
    
    
    void PythonScriptComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
    {
        for (auto& typeName : mDependencies)
        {
            rtti::TypeInfo type = rtti::TypeInfo::get_by_name(typeName);
            
            if (type.is_valid() && type.is_derived_from<Component>())
                components.emplace_back(type);
            else
                nap::Logger::warn("Invalid component dependency typename in %s: %s", mID.c_str(), typeName.c_str());
        }
    }
    
    
    bool PythonScriptComponent::init(utility::ErrorState& errorState)
    {
        mPythonClass = mPythonScript->get(mClassName);
        if (mPythonClass.is_none())
            return false;
            
        return true;
    }

    
    
}
