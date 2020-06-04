#pragma once

// Pybind includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Nap includes
#include <component.h>
#include <entityptr.h>


namespace nap
{

    // Forward declarations
    class Scene;
    class DictVarBase;


    namespace spatial
    {

        // Forward declarations
        class EnvironmentComponentInstance;


        /**
         * Set of instance properties that can be used to pass instance property values to a spawned entity
         */
        class NAPAPI EnvironmentInstanceProperties
        {
            RTTI_ENABLE()
            friend class EnvironmentComponentInstance;

        public:
            EnvironmentInstanceProperties() = default;

            // Copy not allowed
            EnvironmentInstanceProperties(const EnvironmentInstanceProperties&) = delete;
            EnvironmentInstanceProperties& operator=(const EnvironmentInstanceProperties&) = delete;

            /**
             * Add a string instance property to the list.
             * Pass the component type, a path to the property within the component, and the value.
             */
            void addString(const std::string& componentType, const std::string& path, const std::string& value);

            /**
             * Add an integer type instance property to the list.
             * Pass the component type, a path to the int property within the component, and the value.
             */
            void addInt(const std::string& componentType, const std::string& path, int value);

        private:
            class Property
            {
            public:
                Property() = default;

//                // Copy not allowed
//                Property(const Property&) = delete;
//                Property& operator=(const Property&) = delete;

                std::string mComponentType;
                std::string mPath;
                std::unique_ptr<InstancePropertyValue> mValue = nullptr;
            };

        private:
            template <typename T>
            void add(const std::string& componentType, const std::string& path, decltype(T::mValue) value);

            std::vector<std::unique_ptr<Property>> mProperties;
        };


        /**
         * EnvironmentComponent executes an environment script that can initialise a Scene with Entities in runtime.
         */
        class NAPAPI EnvironmentComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(EnvironmentComponent, EnvironmentComponentInstance)
            
        public:
            EnvironmentComponent() : Component() { }
            
            // Inherited from Component
            void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
            
            std::string mScriptPath = "";
            std::vector<EntityPtr> mEntities;

        private:
        };

        
        class NAPAPI EnvironmentComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
        public:
            EnvironmentComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;
            void update(double deltaTime) override;

            EntityInstance* createEntity(const std::string& entityName, EnvironmentInstanceProperties* properties);
            rtti::Object* findResource(const std::string& id);

            void setScriptPath(const std::string& path) { mScriptPath = path; }
            
        private:
            pybind11::module mScript;
            Scene* mScene = nullptr;
            bool mEnvironmentLoaded = false;
            std::string mScriptPath = "";
        };


        template <typename T>
        void EnvironmentInstanceProperties::add(const std::string& componentType, const std::string& path, decltype(T::mValue) value)
        {
            std::unique_ptr<Property> property = std::make_unique<Property>();
            property->mComponentType = componentType;
            property->mPath = path;
            auto instancePropValue = std::make_unique<T>();
            instancePropValue->mID = componentType + "_" + path;
            instancePropValue->mValue = value;
            property->mValue = std::move(instancePropValue);
            mProperties.emplace_back(std::move(property));
        }
    }
        
}
