#pragma once

#include "Shape.h"

#include <Spatial/Core/PythonClassInstanceManager.h>

namespace nap
{
    
    namespace spatial
    {
        
        /**
         * ShapeInstance that calls a python function to apply its transformation.
         * At the moment, the python function only calculates positions (just a port of previous functionality).
         */
        class NAPAPI PythonShapeInstance : public ShapeInstance
        {
            RTTI_ENABLE(ShapeInstance)
            
        public:
            PythonShapeInstance(ShapeBase& shape) : ShapeInstance(shape) { }
            
            virtual std::vector<SpatialTransform> calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time) override;
            
            virtual void update(double deltaTime) override;
            
            virtual void getVerticesForVisualization(std::vector<glm::vec3>& outVertices) override;
            
            virtual bool getEdgesForVisualization(std::vector<std::pair<int,int>> & outEdges) override;
            
        protected:
            virtual void onInit(nap::EntityInstance* entity) override;
            
        private:
            ParameterInt* mParticleCount; ///< external parameter "particle count" used by all python scripts.
            
            PythonClassInstanceManager mPythonClassInstance;
            nap::EntityInstance* mEntity = nullptr;
            
        };
        
        class NAPAPI PythonShape : public Shape<PythonShapeInstance> {
            RTTI_ENABLE(ShapeBase)
            
            friend class PythonShapeInstance;
            
        public:
            PythonShape() = default;
            
            bool init(utility::ErrorState& errorState) override;
            
            ResourcePtr<nap::PythonScript> mPythonScript = nullptr;  ///< property: 'PythonScript' Pointer to a python script resource that manages the script that contains the python class.
            std::string mClassName; ///< property: 'Class' The name of the class defined in the python script
            
        private:
            
            pybind11::object mPythonClass;
            
        };
        
    }

}
