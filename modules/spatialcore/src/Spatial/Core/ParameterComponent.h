#pragma once

#include <parametervec.h>
#include <parametersimple.h>
#include <parameternumeric.h>
#include <parameterenum.h>
#include <Spatial/Utility/ParameterTypes.h>
#include <parameter.h>
#include <component.h>
#include <nap/resourceptr.h>

namespace nap
{

    // Forward declarations
    class ParameterComponentInstance;

    /**
     * Component that allows every instance to manage its own set of parameters.
     * The parameters are kept together in a group for each instance. The groups can be added to an already existing parent group.
     */
    class NAPAPI ParameterComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(ParameterComponent, ParameterComponentInstance)

    public:
        ParameterComponent() : Component() { }

        std::string mName = ""; ///< property: 'Name' The default mID of the instance's parameter group. Recommended to change per instance using instance properties or setting at runtime, so every instance has it's uniquely named group.
        ResourcePtr<ParameterGroup> mParentGroup = nullptr; ///< property: 'ParentGroup' optional parent parametergroup to which this component's group will be added as a child.

    private:

    };


    /**
     * Instance of @ParameterComponent
     */
    class NAPAPI ParameterComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)

    public:
        ParameterComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

        ~ParameterComponentInstance();

        // Initialize the component
        bool init(utility::ErrorState& errorState) override;

        /**
         * Sets the mID of the parameter group managed by the ParameterComponentInstance
         */
        void setGroupName(const std::string& name) { mGroup.mID = name; }
        
        /**
         * Returns the mID of the parameter group managed by the ParameterComponentInstance
         */
        const std::string& getGroupName() const { return mGroup.mID; }

        ParameterFloat& addParameterFloat(const std::string& name, float value, float min, float max) { return addParameterNumeric<float>(name, value, min, max); }
        ParameterInt& addParameterInt(const std::string& name, int value, int min, int max) { return addParameterNumeric<int>(name, value, min, max); }
        ParameterBool& addParameterBool(const std::string& name, bool value) { return addParameterSimple<bool>(name, value); }
        ParameterVec2& addParameterVec2(const std::string& name, glm::vec2 value, float min, float max) { return addParameterVec<glm::vec2>(name, value, min, max); }
        ParameterVec3& addParameterVec3(const std::string& name, glm::vec3 value, float min, float max) { return addParameterVec<glm::vec3>(name, value, min, max); }
        
        ParameterString& addParameterString(const std::string& name, std::string value) { return addParameterSimple<std::string>(name, value); }

        /**
         * Creates a new ParameterSimple of value type T and returns a reference to it.
         * Emits the @parameterCreated signal after setting the parameter's metadata.
         */
        template <typename T>
        ParameterSimple<T>& addParameterSimple(const std::string& name, const T& value)
        {
            auto& parameter = addParameter<ParameterSimple<T>, T>(name, value);
            parameterCreated(parameter);
            return parameter;
        }

        /**
         * Creates a new ParameterNumeric of numeric value type T and returns a reference to it.
         * Emits the @parameterCreated signal after setting the parameter's metadata.
         */
        template <typename T>
        ParameterNumeric<T>& addParameterNumeric(const std::string& name, const T& value, const T& min, const T& max)
        {
            auto& parameter = addParameter<ParameterNumeric<T>, T>(name, value);
            parameter.setRange(min, max);
            parameterCreated(parameter);
            return parameter;
        }

        /**
         * Creates a new ParameterEnum of enum type T and returns a reference to it.
         * Emits the @parameterCreated signal.
         */
        template <typename T>
        ParameterEnum<T>& addParameterEnum(const std::string& name, const T& value)
        {
            auto& parameter = addParameter<ParameterEnum<T>, T>(name, value);
            parameterCreated(parameter);
            return parameter;
        }

        /**
         * Creates a new ParameterEnum of enum type T and returns a reference to it.
         * Emits the @parameterCreated signal after setting the parameter's metadata.
         */
        template <typename T>
        ParameterVec<T>& addParameterVec(const std::string& name, const T& value, const float& min, const float& max)
        {
            auto& parameter = addParameter<ParameterVec<T>, T>(name, value);
            parameter.setRange(min, max);
            parameter.mClamp = true;
            parameterCreated(parameter);
            return parameter;
        }


        /**
         * Creates a new parameter of type ParameterType and sets it's value. Assumes that ParameterType has a method setValue() that takes a ValueType as its only argument.
         * The name of the newly created parameter will be the given 'name'.
         * The mID of the newly created parameter will be the given 'name' prefixed by '[group mID]/'.
         * Returns a reference to the newly created parameter.
         * Does not emit the parameterCreated signal yet, as the parameter's metadata have not yet been set.
         */
        template <typename ParameterType, typename ValueType>
        ParameterType& addParameter(const std::string& name, const ValueType& value)
        {
            // Make sure no parameter with this name exists already
            assert(findParameter(name) == nullptr);

            auto parameter = std::make_unique<ParameterType>();
            std::string prefixedName = mGroup.mID + "/" + name;
            parameter->mID = prefixedName;
            parameter->mName = name;
            parameter->mValue = value;
            auto parameterPtr = parameter.get();
            mGroup.mParameters.emplace_back(parameterPtr);
            mParameters.emplace_back(std::move(parameter));
            return *parameterPtr;
        }

        /**
         * Finds a parameter in the group by name.
         */
        Parameter* findParameter(const std::string& name)
        {
            auto it = std::find_if(mParameters.begin(), mParameters.end(), [&](auto& param){ return param->mName == name; });

            if (it != mParameters.end())
                return it->get();
            else
                return nullptr;
        }

        template <typename ParameterType>
        ParameterType* findParameter(const std::string& name)
        {
            return rtti_cast<ParameterType>(findParameter(name));
        }

        /**
         * Returns the number of existing parameters.
         */
        int getNumberOfParameters() const { return mParameters.size(); }
        
        const ParameterGroup& getGroup() const { return mGroup; }
        ParameterGroup& getGroup() { return mGroup; }

        /**
         * Signal that is emitted each time a new parameter has been added to the component.
         */
        Signal<Parameter&> parameterCreated;
        
    private:
        ParameterGroup mGroup;
        std::vector<std::unique_ptr<Parameter>> mParameters;
        std::vector<std::unique_ptr<ParameterGroup>> mChildGroups;
        ResourcePtr<ParameterGroup> mParentGroup = nullptr;
    };
    
}
