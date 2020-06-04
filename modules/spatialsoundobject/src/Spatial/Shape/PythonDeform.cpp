
#include "PythonDeform.h"

#include <Spatial/Core/SpatialTypes.h>

RTTI_BEGIN_CLASS(nap::spatial::PythonDeform)
    RTTI_PROPERTY("PythonScript", &nap::spatial::PythonDeform::mPythonScript, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Class", &nap::spatial::PythonDeform::mClassName, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::PythonDeformInstance)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        bool PythonDeform::init(utility::ErrorState& errorState)
        {
            mPythonClass = mPythonScript->get(mClassName);
            if (mPythonClass.is_none())
                return false;
            
            return true;
        }
        
        
        void PythonDeformInstance::onInit(nap::EntityInstance* entity)
        {
            auto* resource = getDeform<PythonDeform>();
            mPythonClassInstance.init(resource->mPythonScript.get(), &resource->mPythonClass, entity, getName(), &getParameterManager());
            
            mEntity = entity;
            
        }
        
        
        void PythonDeformInstance::update(double deltaTime)
        {
            mPythonClassInstance.call<void>("update", deltaTime);
        }
        
        
        void PythonDeformInstance::apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform)
        {
            std::vector<SpatialTransform> resultTransforms = mPythonClassInstance.call<std::vector<SpatialTransform>, std::vector<SpatialTransform>>("apply", transforms);
            
            if(resultTransforms.size() != transforms.size())
            {
                nap::Logger::warn("PythonDeform's returned particle count is incorrect.");
                return;
            }
            
            transforms = resultTransforms;
        }
     
    }
}
