#include "OSCParameterControlComponent.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Utility/ParameterTypes.h>

// Nap includes
#include <oscinputcomponent.h>
#include <entity.h>
#include <nap/logger.h>

// RTTI
RTTI_BEGIN_CLASS(nap::spatial::OSCParameterControlComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::OSCParameterControlComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

    namespace spatial
    {

        void OSCParameterControlComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.emplace_back(RTTI_OF(OSCInputComponent));
            components.emplace_back(RTTI_OF(ParameterComponent));
        }


        bool OSCParameterControlComponentInstance::init(utility::ErrorState& errorState)
        {
            registerDefaultParameterTypes();

            mParameterComponent = getEntityInstance()->findComponent<ParameterComponentInstance>();
            if(!errorState.check(mParameterComponent != nullptr, "ParameterComponent not found."))
                return false;
            mOSCInputComponent = getEntityInstance()->findComponent<OSCInputComponentInstance>();
            if(!errorState.check(mOSCInputComponent != nullptr, "OSCInputComponent not found."))
               return false;
            
            // Add OSC handlers for already existing parameters
            for (auto& parameter : mParameterComponent->getGroup().mParameters)
                parameterCreated(*parameter);

            // Add parametergroup name as OSC address filter
            mOSCInputComponent->mAddressFilter.emplace_back("/" + mParameterComponent->getGroupName());

            mParameterComponent->parameterCreated.connect(parameterCreatedSlot);
            mOSCInputComponent->messageReceived.connect(oscMessageReceivedSlot);

            return true;
        }


        void OSCParameterControlComponentInstance::parameterCreated(Parameter &parameter)
        {
            auto it = mInputHandlerCreators.find(parameter.get_type());
            if (it == mInputHandlerCreators.end())
            {
                Logger::warn("No OSC input support for parameter type: %s", parameter.get_type().get_name().to_string().c_str());
                return;
            }
            mOSCInputHandlers["/" + parameter.mID] = mInputHandlerCreators[parameter.get_type()](parameter);
        }


        void OSCParameterControlComponentInstance::oscMessageReceived(const nap::OSCEvent &event)
        {
            auto it = mOSCInputHandlers.find(event.getAddress());
            if (it == mOSCInputHandlers.end())
            {
                unknownOSCMessageReceived(event);
                return;
            }
            if (!it->second(event))
            {
                conversionErrorOccurred(event);
                return;
            }
            knownOSCMessageReceived(event);
        }



        void OSCParameterControlComponentInstance::registerDefaultParameterTypes()
        {
            registerParameterType<ParameterFloat>([](const OSCEvent& event, float& value){
                return eventToNumeric<float>(event, value);
            });

            registerParameterType<ParameterInt>([](const OSCEvent& event, int& value){
                return eventToNumeric<int>(event, value);
            });

            registerParameterType<ParameterOptionList>([](const OSCEvent& event, ParameterOptionList& parameter){
                if (event.getCount() < 1)
                    return false;
                auto argument = event.getArgument(0);
                if (argument->isString())
                    return parameter.setOption(argument->asString());
                return false;
            });

            registerParameterType<ParameterBool>([](const OSCEvent& event, bool& value){
                if (event.getCount() == 0)
                    return false;
                auto argument = event.getArgument(0);
                if (argument->isBool())
                {
                    value = argument->asBool();
                    return true;
                }
                else if (argument->isInt()) // also accept integers as input for bool parameters
                {
                    value = argument->asInt();
                    return true;
                }
                return false;
            });

            registerParameterType<ParameterString>([](const OSCEvent& event, std::string& value){
                if (event.getCount() == 0)
                    return false;
                auto argument = event.getArgument(0);
                if (argument->isString())
                {
                    value = argument->asString();
                    return true;
                }
                return false;
            });

            registerParameterType<ParameterVec2>([](const OSCEvent& event, glm::vec2& value){
                if (event.getCount() != 2)
                    return false;
                auto arg1 = event.getArgument(0);
                auto arg2 = event.getArgument(1);

                if (arg1->isFloat() && arg2->isFloat())
                {
                    value = glm::vec2(arg1->asFloat(), arg2->asFloat());
                    return true;
                }

                return false;
            });

            registerParameterType<ParameterVec3>([](const OSCEvent& event, glm::vec3& value){
                if (event.getCount() != 3)
                    return false;
                auto arg1 = event.getArgument(0);
                auto arg2 = event.getArgument(1);
                auto arg3 = event.getArgument(2);

                if (arg1->isFloat() && arg2->isFloat() && arg3->isFloat())
                {
                    value = glm::vec3(arg1->asFloat(), arg2->asFloat(), arg3->asFloat());
                    return true;
                }

                return false;
            });
        }

    }

}
