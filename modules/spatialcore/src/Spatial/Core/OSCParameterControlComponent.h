#pragma once

#include <component.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <oscevent.h>

namespace nap
{

    // Forward declarations
    class OSCInputComponentInstance;
    class ParameterComponentInstance;
    class Parameter;

    namespace spatial
    {

        // Forward declarations
        class OSCParameterControlComponentInstance;


        /**
         * Component to rout OSC input from @OSCInputComponent to parameters managed by @ParameterComponent.
         * By default uses the mID of parameters as OSC address.
         */
        class NAPAPI OSCParameterControlComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(OSCParameterControlComponent, OSCParameterControlComponentInstance)

        public:
            OSCParameterControlComponent() = default;

            // Inherited from Component
            void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

        private:

        };


        /**
         * Instance of @OSCParameterControlComponent
         */
        class NAPAPI OSCParameterControlComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)

        public:
            OSCParameterControlComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
            ~OSCParameterControlComponentInstance() override { }

            // Initialize the component
            bool init(utility::ErrorState& errorState) override;

            /**
             * All parameter types available for OSC control should be registered by this method.
             * This method takes a converter function that sets a parameter value from an OSCEvent's arguments.
             * @tparam T The parameter type to be supported
             * @param converter A function that takes an @OSCEvent and a parameter of the type to be supported as arguments. The function should try to set the parameter's value from the content of the OSCEvent's arguments. It should return true on success.
             */
            template <typename T>
            void registerParameterType(std::function<bool(const OSCEvent&, T& parameter)> converter);

            /**
             * Overload of the more generic registerParameterType.
             * @tparam T he parameter type to be supported
             * @param converter A function that takes an @OSCEvent and a value by reference. The converter should try to set the value using the OSCEvent's arguments and return true on success.
             */
            template <typename T>
            void registerParameterType(std::function<bool(const OSCEvent&, decltype(T::mValue)& value)> converter);

            /**
             * Signal emitted every time an OSC message is matched with a parameter and the parameter is sucessfully set.
             */
            Signal<const OSCEvent&> knownOSCMessageReceived;

            /**
             * Signal emitted every time an OSC message that doesn't match with a registered parameter is received and the parameter is set succesfully.
             */
            Signal<const OSCEvent&> unknownOSCMessageReceived;

            /**
             * Signal emitted every an OSC message has been matched with a parameter but fails to convert to the parameter's data type.
             */
            Signal<const OSCEvent&> conversionErrorOccurred;

        private:
            // Function that handles an incoming OSCEvent. Is sets a parameter's value and returns true on sucess.
            using OSCInputHandler = std::function<bool(const OSCEvent&)>;

            // Function that creates an @OSCInputHandler function for a certain parameter type. Functions of this type are created by the @registerParameterType() method and retained in a map paired with the parameter type.
            using OSCInputHandlerCreator = std::function<OSCInputHandler(Parameter&)>;

        private:
            ParameterComponentInstance* mParameterComponent = nullptr;
            OSCInputComponentInstance* mOSCInputComponent = nullptr;

            // Map of @OSCInputHandlerCreator functions paired with the parameter type they belong to. For each registered parameter type there is a OSCInputHandlerCreator function that can create an input handler for one specific parameter.
            std::unordered_map<rtti::TypeInfo, OSCInputHandlerCreator> mInputHandlerCreators;

            // Map of @OSCInputHandler functions for each parameter controlled by this component. The handlers are paired with an OSC address. For incoming OSCEvents that match this address the OSCInputHandler will be invoked.
            std::unordered_map<std::string, OSCInputHandler> mOSCInputHandlers;

            Slot<Parameter&> parameterCreatedSlot = { this, &OSCParameterControlComponentInstance::parameterCreated };
            void parameterCreated(Parameter& parameter);

            Slot<const OSCEvent&> oscMessageReceivedSlot = { this, &OSCParameterControlComponentInstance::oscMessageReceived };
            void oscMessageReceived(const OSCEvent& event);

        private:
            // Helper function that tries to convert the event's first argument to a numeric type T. It can perform cross conversion between float, double and int. It returns true on sucess.
            template <typename T>
            static bool eventToNumeric(const OSCEvent& event, T& value);

            void registerDefaultParameterTypes();
        };


        template <typename T>
        void OSCParameterControlComponentInstance::registerParameterType(std::function<bool(const OSCEvent&, T& parameter)> converter)
        {
            mInputHandlerCreators[RTTI_OF(T)] = [converter](Parameter& parameter)
            {
                auto parameterPtr = &parameter;
                return [parameterPtr, converter](const OSCEvent& event)->bool
                {
                    return converter(event, *rtti_cast<T>(parameterPtr));
                };
            };
        }

        template <typename T>
        void OSCParameterControlComponentInstance::registerParameterType(std::function<bool(const OSCEvent&, decltype(T::mValue)& value)> converter)
        {
            mInputHandlerCreators[RTTI_OF(T)] = [converter](Parameter& parameter)
            {
                auto parameterPtr = &parameter;
                return [parameterPtr, converter](const OSCEvent& event)->bool
                {
                    auto typedParameter = rtti_cast<T>(parameterPtr);
                    auto value = typedParameter->mValue;
                    if (converter(event, value))
                    {
                        typedParameter->setValue(value);
                        return true;
                    }
                    return false;
                };
            };
        }


        template <typename T>
        bool OSCParameterControlComponentInstance::eventToNumeric(const OSCEvent& event, T& value)
        {
            if (event.getCount() == 0)
                return false;

            auto argument = event.getArgument(0);
            if (argument->isFloat())
            {
                value = argument->asFloat();
                return true;
            }
            else if (argument->isDouble())
            {
                value = argument->asDouble();
                return true;
            }
            else if (argument->isInt())
            {
                value = argument->asInt();
                return true;
            }
            return false;
        }


    }




}