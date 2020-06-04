#include "ParameterComponent.h"

#include <entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::ParameterComponent)
    RTTI_PROPERTY("Name", &nap::ParameterComponent::mName, nap::rtti::EPropertyMetaData::Required)

RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("findParameter", static_cast<nap::Parameter* (nap::ParameterComponentInstance::*)(const std::string&)>(&nap::ParameterComponentInstance::findParameter))
RTTI_END_CLASS


namespace nap
{

    ParameterComponentInstance::~ParameterComponentInstance()
    {
        // Remove group from parent group.
        if (mParentGroup != nullptr)
        {
            auto it = std::find_if(mParentGroup->mChildren.begin(), mParentGroup->mChildren.end(), [&](auto& ptr){ return ptr.get() == &mGroup; });
            if (it != mParentGroup->mChildren.end())
                mParentGroup->mChildren.erase(it);
        }
    }




    bool ParameterComponentInstance::init(utility::ErrorState& errorState)
    {
        auto component = getComponent<ParameterComponent>();
        mGroup.mID = component->mName;
        mParentGroup = component->mParentGroup;

        mParentGroup->mChildren.emplace_back(&mGroup);

        return true;
    }


}

