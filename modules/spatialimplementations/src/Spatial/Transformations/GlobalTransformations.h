#pragma once

// Spatial includes
#include <Spatial/Transformation/Transformation.h>

namespace nap
{
    namespace spatial
    {

        
        /**
         * Transformation that multiplies the position by a vec3 parameter in relation to the anchorpoint.
         */
        class NAPAPI PlodeTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            PlodeTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
            
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterVec3* mPlode = nullptr;
            
        };
        
        DECLARE_TRANSFORMATION(PlodeTransformation, PlodeTransformationInstance)
        
        
        /**
         * Transformation that multiplies the position by a float parameter in all directions, in relation to the anchorpoint.
         */
        class NAPAPI PlodeScaleTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            PlodeScaleTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
            
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterFloat* mPlodeScale = nullptr;
            
        };
        
        DECLARE_TRANSFORMATION(PlodeScaleTransformation, PlodeScaleTransformationInstance)
        
        
        /**
         * Transformation that multiplies the dimensions and the position in relation to the anchorpoint.
         */
        class NAPAPI StretchTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            StretchTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
            
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterVec3* mStretch = nullptr;
            
        };
        
        DECLARE_TRANSFORMATION(StretchTransformation, StretchTransformationInstance)

        
        /**
         * Transformation that multiplies the dimensions and the position in relation to the anchorpoint.
         */
        class NAPAPI StretchScaleTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            StretchScaleTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
            
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterFloat* mStretchScale = nullptr;
            
        };
        
        DECLARE_TRANSFORMATION(StretchScaleTransformation, StretchScaleTransformationInstance)
        
        
        /**
         * Transformation that rotates around the anchorpoint.
         */
        class NAPAPI RotationTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            RotationTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
            
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterVec3* mRotation = nullptr;
            
        };
        
        DECLARE_TRANSFORMATION(RotationTransformation, RotationTransformationInstance)

    }
}
