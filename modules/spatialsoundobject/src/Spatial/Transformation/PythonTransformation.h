#pragma once

// Spatial includes
#include <Spatial/Transformation/Transformation.h>
#include <Spatial/Core/PythonClassInstanceManager.h>

namespace nap {
    namespace spatial {
    
        
        class NAPAPI PythonTransformationInstance : public TransformationInstance {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            PythonTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation){ }

            void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;

            virtual void update(double deltaTime) override;
            
        private:
            PythonClassInstanceManager mPythonManager;
            
            ParameterBool* mEnable = nullptr;
        };
        
        
        class NAPAPI PythonTransformation : public Transformation<PythonTransformationInstance> {
            RTTI_ENABLE(TransformationBase)
        public:
            friend class PythonTransformationInstance;
            
            PythonTransformation() : Transformation<PythonTransformationInstance>() { }
            
            bool init(utility::ErrorState& errorState);

            ResourcePtr<nap::PythonScript> mPythonScript = nullptr;  ///< property: 'PythonScript' Pointer to a python script resource that manages the script that contains the python class for this component.
            std::string mClassName; ///< property: 'Class' The name of the class defined in the python script
            
        private:
            pybind11::object mPythonClass;

        };
        
    }
}
