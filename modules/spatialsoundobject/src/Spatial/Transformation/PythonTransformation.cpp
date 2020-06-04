
#include "PythonTransformation.h"

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Core/ParameterComponent.h>


RTTI_BEGIN_CLASS(nap::spatial::PythonTransformation)
    RTTI_PROPERTY("PythonScript", &nap::spatial::PythonTransformation::mPythonScript, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Class", &nap::spatial::PythonTransformation::mClassName, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::PythonTransformationInstance)
RTTI_END_CLASS


namespace nap
{
    namespace spatial
    {
        
        bool PythonTransformation::init(utility::ErrorState& errorState)
        {
            mPythonClass = mPythonScript->get(mClassName);
            if (mPythonClass.is_none())
                return false;
            
            return true;
        }
        
        void PythonTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            auto* resource = getTransformation<PythonTransformation>();
            mPythonManager.init(resource->mPythonScript.get(), &resource->mPythonClass, entity, getName(), &getParameterManager());
            
            mEnable = getParameterManager().addParameterBool("enable", false);
            
        }
        
        void PythonTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            if(mEnable->mValue){
                SpatialTransform transform = mPythonManager.call<SpatialTransform, SpatialTransform>("apply", SpatialTransform(position, scale, rotation));
                position = transform.mPosition;
                scale = transform.mScale;
                rotation = transform.mRotation;
            }
        }

        void PythonTransformationInstance::update(double deltaTime)
        {
            if(mEnable->mValue){
                mPythonManager.call<void>("update", deltaTime);
            }
        }
        
    }
}
