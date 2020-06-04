#include "EnvironmentComponent.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>

// Nap includes
#include <pythonscriptservice.h>
#include <scene.h>
#include <nap/core.h>
#include <nap/logger.h>


// RTTI
RTTI_BEGIN_CLASS(nap::spatial::EnvironmentInstanceProperties)
    RTTI_FUNCTION("addString", &nap::spatial::EnvironmentInstanceProperties::addString)
    RTTI_FUNCTION("addInt", &nap::spatial::EnvironmentInstanceProperties::addInt)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::spatial::EnvironmentComponent)
    RTTI_PROPERTY("ScriptPath", &nap::spatial::EnvironmentComponent::mScriptPath, nap::rtti::EPropertyMetaData::FileLink)
    RTTI_PROPERTY("Entities", &nap::spatial::EnvironmentComponent::mEntities, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::EnvironmentComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("createEntity", &nap::spatial::EnvironmentComponentInstance::createEntity)
    RTTI_FUNCTION("findResource", &nap::spatial::EnvironmentComponentInstance::findResource)
RTTI_END_CLASS

namespace nap
{

    namespace spatial
    {

        void EnvironmentInstanceProperties::addString(const std::string& componentType, const std::string& path, const std::string& value)
        {
            add<StringInstancePropertyValue>(componentType, path, value);
        }


        void EnvironmentInstanceProperties::addInt(const std::string& componentType, const std::string& path, int value)
        {
            add<Int32InstancePropertyValue>(componentType, path, value);
        }


        void EnvironmentComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.emplace_back(RTTI_OF(ParameterComponent));
        }


        bool EnvironmentComponentInstance::init(utility::ErrorState& errorState)
        {
            auto resource = getComponent<EnvironmentComponent>();
            mScriptPath = resource->mScriptPath;

            return true;
        }


        void EnvironmentComponentInstance::update(double deltaTime)
        {
            if (!mEnvironmentLoaded)
            {
                mEnvironmentLoaded =true;

                // Find the scene
                mScene = getEntityInstance()->getCore()->getResourceManager()->findObject<Scene>("Scene").get();
                if (mScene == nullptr)
                {
                    nap::Logger::info("Environment error: Scene not found: %s", "Scene");
                    return;
                }

                // Load the script
                auto scriptService = getEntityInstance()->getCore()->getService<PythonScriptService>();
                utility::ErrorState errorState;
                if (!scriptService->TryLoad(mScriptPath, mScript, errorState))
                {
                    nap::Logger::fatal("Failed to load environment script %s", mScriptPath.c_str());
                    return;
                }

                // Call ths script's init callback
                try
                {
                    mScript.attr("init")(getEntityInstance());
                }
                catch (const pybind11::error_already_set& err)
                {
                    nap::Logger::info("Runtime python error while executing %s: %s", mScriptPath.c_str(), err.what());
                }
            }
        }


        EntityInstance* EnvironmentComponentInstance::createEntity(const std::string& entityName, EnvironmentInstanceProperties* properties)
        {
            auto entity = getEntityInstance()->getCore()->getResourceManager()->findObject<Entity>(entityName);

            if (entity == nullptr)
            {
                Logger::warn("Failed to find entity %s", entityName.c_str());
                return nullptr;
            }

            utility::ErrorState errorState;

            std::vector<ComponentInstanceProperties> componentInstancePropertiesList;
            for (auto& property : properties->mProperties)
            {
                auto component = entity->findComponent(rtti::TypeInfo::get_by_name(property->mComponentType));
                if (component == nullptr)
                {
                    Logger::warn("Component not found: %s", property->mComponentType.c_str());
                    return nullptr;
                }

                ComponentInstanceProperties* componentInstanceProperties = nullptr;
                auto it = std::find_if(componentInstancePropertiesList.begin(), componentInstancePropertiesList.end(), [&](ComponentInstanceProperties& compProps){ return compProps.mTargetComponent.get() == component.get(); });

                if (it == componentInstancePropertiesList.end())
                {
                    ComponentInstanceProperties newComponentInstanceProperties;
                    newComponentInstanceProperties.mTargetComponent.assign("./" + component->mID, *component);
                    componentInstancePropertiesList.emplace_back(newComponentInstanceProperties);
                    componentInstanceProperties = &componentInstancePropertiesList.back();
                }
                else
                    componentInstanceProperties = &(*it);

                TargetAttribute targetAttribute;
                targetAttribute.mPath = property->mPath;
                targetAttribute.mValue = property->mValue.get();
                componentInstanceProperties->mTargetAttributes.emplace_back(targetAttribute);
            }

            SpawnedEntityInstance spawnedInstance = mScene->spawn(*entity, componentInstancePropertiesList, errorState);
            if (errorState.toString() != "")
            {
                Logger::warn("Failed to spawn entity %s: %s", entityName.c_str(), errorState.toString().c_str());
                return nullptr;
            }

            return spawnedInstance.get().get();
        }


        rtti::Object* EnvironmentComponentInstance::findResource(const std::string& id)
        {
//            return getEntityInstance()->getCore()->getResourceManager()->findObject(id);
            auto result = getEntityInstance()->getCore()->getResourceManager()->findObject(id);
            if (result != nullptr)
                return result.get();
            else
                return nullptr;
        }

    }

}
