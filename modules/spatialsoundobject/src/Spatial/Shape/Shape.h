#pragma once

// Spatial includes
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Utility/ParameterTypes.h>
#include <Spatial/Core/ParameterManager.h>

// NAP includes
#include <nap/resource.h>
#include <entity.h>

// Macro to define a Shape named Type from instance InstanceType.
#define DECLARE_SHAPE(Type, InstanceType)  class NAPAPI Type : public Shape<InstanceType> { RTTI_ENABLE(ShapeBase) };


namespace nap
{

    // forward declarations
    class ParameterComponentInstance;

    namespace spatial
    {

        class ShapeInstance;
        
        /**
         * Base class so we can rtti-define templated @Shape resources.
         */
        class NAPAPI ShapeBase : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            ShapeBase() = default;
            
            std::string mName; ///< property: 'Name' Used to set parameter prefixes.
            
            virtual std::unique_ptr<ShapeInstance> instantiate(nap::utility::ErrorState& errorState, nap::EntityInstance* entity) { return nullptr; }
            
        };
        
        
        /**
         * A @Shape is a resource that can instantiate @ShapeInstance instances.
         */
        template <typename InstanceType>
        class NAPAPI Shape : public ShapeBase
        {
            RTTI_ENABLE(ShapeBase)
            
        public:
            Shape() = default;
            
            /**
             * Instantiates and initialises a ShapeInstance. Returns nullptr if initialisation failed.
             */
            std::unique_ptr<ShapeInstance> instantiate(nap::utility::ErrorState& errorState, nap::EntityInstance* entity) override
            {
                auto instance = std::make_unique<InstanceType>(*this);
                bool initSucceeded = instance->init(errorState, entity);
                if(initSucceeded)
                    return std::move(instance);
                return nullptr;
                
            }
        };

        

        /**
         * A shape outputs a number of particle transforms based on the the sound object's @SpatialTransform, the desired @particleCount and the @time. The size of the output vector doesn't have to be equal to the desired @particleCount. The @time value is local to the shape.
         */
        class NAPAPI ShapeInstance
        {
            
            RTTI_ENABLE()
        public:
            
            ShapeInstance(ShapeBase& shape) : mShape(shape) { }

            virtual ~ShapeInstance() = default;
            
            virtual bool init(nap::utility::ErrorState& errorState, nap::EntityInstance* entity);
            
            /**
             * Calculates the transforms of the currently active particles in the shape.
             * @param soundObjectTransform  the transform of the sound object as a whole.
             * @param time in seconds, but can be relative to the speed parameter in the @ShapeComponent.
             * @return vector of transforms of all the particles that are currently active at the provided @time.
             */
            virtual std::vector<SpatialTransform> calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time) = 0;
            
            /**
             * Will be called by the @ShapeComponent on every update cycle.
             * @param deltaTime in seconds
             */
            virtual void update(double deltaTime) { }

            /**
             * @return name of this shape as specified in its resource.
             */
            const std::string& getName() const { return mShape.mName; }

            /**
             * Sets vertices for visualisation. Vertices are in object space (scaling needs to be cancelled out as it will be applied automatically).
             */
            virtual void getVerticesForVisualization(std::vector<glm::vec3>& outVertices) { }
        
            /**
             * Sets edges for visualisation.
             */
            virtual bool getEdgesForVisualization(std::vector<std::pair<int,int>> & outEdges)
            {
                return false;
            }
            
            
        protected:
            /**
             * Virtual function called during initialisation. Can be used to set up parameters etc.
             */
            virtual void onInit(nap::EntityInstance* entity) { };
                        
            /**
             * Returns a pointer to the resource.
             */
            template <typename T>
            T* getShape() { return rtti_cast<T>(&mShape); };

            ParameterManager& getParameterManager() { return mParameterManager; }
            
        private:
            ParameterManager mParameterManager;
            
            ShapeBase& mShape; // Reference to the resource of this shape instance.
            
        };
        
    }
    
}
