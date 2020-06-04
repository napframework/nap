#pragma once

#include "Deform.h"

#include <Spatial/Core/PythonClassInstanceManager.h>

namespace nap
{
    namespace spatial
    {
        
        /**
         * DeformInstance that calls a python function to apply its transformation.
         * At the moment, the python function only calculates positions (just a port of previous functionality).
         */
        class NAPAPI PythonDeformInstance : public DeformInstance
        {
            RTTI_ENABLE(DeformInstance)
            
        public:
            PythonDeformInstance(DeformBase& deform) : DeformInstance(deform) { }
            
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(std::vector<SpatialTransform>& transforms, const SpatialTransform& soundObjectTransform) override;
            
            virtual void update(double deltaTime) override;
            
        private:
            PythonClassInstanceManager mPythonClassInstance;
            nap::EntityInstance* mEntity = nullptr;
            
        };
        
        class NAPAPI PythonDeform : public Deform<PythonDeformInstance> {
            RTTI_ENABLE(DeformBase)
            friend class PythonDeformInstance;
            
        public:
            PythonDeform() = default;
            
            bool init(utility::ErrorState& errorState) override;
            
            ResourcePtr<nap::PythonScript> mPythonScript = nullptr;  ///< property: 'PythonScript' Pointer to a python script resource that manages the script that contains the python class.
            std::string mClassName; ///< property: 'Class' The name of the class defined in the python script
            
        private:
            pybind11::object mPythonClass;
            
        };
    }
}
