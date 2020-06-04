#include "ExposedDataComponent.h"

#include <rtti/rttiutilities.h>

#include <nap/logger.h>


RTTI_BEGIN_CLASS(nap::spatial::ExposedDataComponent)
    RTTI_PROPERTY("OscSender", &nap::spatial::ExposedDataComponent::mOscSender, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::ExposedDataComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("addOSCOutput", &nap::spatial::ExposedDataComponentInstance::addOSCOutputByName)
RTTI_END_CLASS

namespace nap
{

    namespace spatial
    {

        void ExposedDataComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.push_back(RTTI_OF(ParameterComponent));
        }
        
        
        
        bool ExposedDataComponentInstance::init(utility::ErrorState &errorState)
        {
            mOSCSender = getComponent<ExposedDataComponent>()->mOscSender;

            mOSCAddressPrefix = "/"; // all osx address prefixes start with "/"
            auto parameterComponent = getEntityInstance()->findComponent<ParameterComponentInstance>();
            if (parameterComponent != nullptr)
            {
                const std::string& name = parameterComponent->getGroupName();
                mOSCAddressPrefix.append(name + "/");
                
                parameterComponent->parameterCreated.connect(mParameterCreatedSlot);
            }

            registerDefaultExposeParameterFunctions();
            
            return true;
        }


        void ExposedDataComponentInstance::update(double deltaTime)
        {
            if (mOSCSender != nullptr)
            {
                bool sending = false;
                for (auto &oscOutput : mOSCOutputs)
                {
                    auto event = oscOutput->createOSCEvent(mOSCAddressPrefix);
                    if (event != nullptr)
                    {
                        mOSCSender->addEvent(std::move(event));
                        sending = true;
                    }
                }
                if (sending)
                    mOSCSender->sendQueuedEvents();
            }
        }

        
        void ExposedDataComponentInstance::registerExposeParameterFunction(const rtti::TypeInfo& type, const ExposeParameterFunction& exposeParameterFunction)
        {
            std::vector<rtti::TypeInfo> types;
            types.push_back(type);
            rtti::getDerivedTypesRecursive(type, types);
            
            for (const rtti::TypeInfo& type : types)
                mExposeParameterFunctions[type] = exposeParameterFunction;
        }

        
        void ExposedDataComponentInstance::registerDefaultExposeParameterFunctions()
        {
            registerParameterType<ParameterFloat>();
            registerParameterType<ParameterDouble>();
            registerParameterType<ParameterInt>();
            registerParameterType<ParameterBool>();
            registerParameterType<ParameterVec2>();
            registerParameterType<ParameterVec3>();
            registerParameterType<ParameterOptionList>();
            registerParameterType<ParameterString>();
        }
        
        void ExposedDataComponentInstance::parameterCreated(Parameter& parameter)
        {
            const rtti::TypeInfo& type = parameter.get_type();
            ExposeParameterFunctionMap::iterator pos = mExposeParameterFunctions.find(type);
            if(pos != mExposeParameterFunctions.end())
                pos->second(parameter);
            else
                nap::Logger::warn("Parameter %s not exposed. Type not supported.");
        }


        void ExposedDataComponentInstance::addOSCOutputByName(const std::string& name)
        {
            auto it = mExposedData.getValues().find(name);
            if (it != mExposedData.getValues().end())
                addOSCOutput(*it->second);
        }

    }

}
