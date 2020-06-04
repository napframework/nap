#pragma once

// Spatial includes
#include <Spatial/Transformation/Transformation.h>

namespace nap
{
    
    namespace spatial
    {
                
        /**
         * Input position.
         */
        class NAPAPI InputPositionTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            InputPositionTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
            
            void onInit(nap::EntityInstance* entity) override;
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterVec3* mInputPosition = nullptr;
        };
        
        DECLARE_TRANSFORMATION(InputPositionTransformation, InputPositionTransformationInstance)

        

        /**
         * Input dimensions.
         */
        class NAPAPI InputDimensionsTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            InputDimensionsTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }

            void onInit(nap::EntityInstance* entity) override;
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterVec3* mInputDimensions = nullptr;
        };
        
        
        /**
         * Resource class with added property for adjustable default value.
         */
        
        class NAPAPI InputDimensionsTransformation : public Transformation<InputDimensionsTransformationInstance>
        {
            RTTI_ENABLE(TransformationBase)
            
        public:
            InputDimensionsTransformation() : Transformation<InputDimensionsTransformationInstance>() { }
            
            float mDefaultValue = 1.;  ///< property: The default value of the 'dimensions' parameter.
        };
        
        
        /**
         * Input scale.
         */
        class NAPAPI InputScaleTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            InputScaleTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
            
            void onInit(nap::EntityInstance* entity) override;
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterFloat* mInputScale = nullptr;
        };
        
        
        /** 
         * Resource class with added property for adjustable default value.
         */
        
        class NAPAPI InputScaleTransformation : public Transformation<InputScaleTransformationInstance>
        {
            RTTI_ENABLE(TransformationBase)
            
        public:
            InputScaleTransformation() : Transformation<InputScaleTransformationInstance>() { }
            
            float mDefaultValue = 1.;  ///< property: The default value of the 'scale' parameter.
        };
        
        /**
         * Input orientation.
         * This needs to be chained at the end of the chain, because it needs to work based on the actual position.
         */
        class NAPAPI OrientationTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            OrientationTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) {}
            
            void onInit(nap::EntityInstance* entity) override;
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterVec3* mInputRotation;
            ParameterOptionList* mRotationMode;
            
            // circular buffer to keep track of last x positions for movement orientation
            int mLastPositionsIndex = 0;
            static const int NUM_LAST_POSITIONS = 25;
            std::array<glm::vec3, NUM_LAST_POSITIONS> mLastPositions;
            
        };
        
        DECLARE_TRANSFORMATION(OrientationTransformation, OrientationTransformationInstance)
        
        
        /**
         * Position rotation.
         */
        class NAPAPI PositionRotationTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            PositionRotationTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) {}
            
            void onInit(nap::EntityInstance* entity) override;
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterVec3* mPositionRotation;
            
        };
        
        DECLARE_TRANSFORMATION(PositionRotationTransformation, PositionRotationTransformationInstance)

        
        /**
         * Invert.
         */
        class NAPAPI InvertTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
        public:
            InvertTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) {}
            
            void onInit(nap::EntityInstance* entity) override;
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
        private:
            ParameterBool* mInvertX;
            ParameterBool* mInvertY;
            ParameterBool* mInvertZ;
        };
        
        DECLARE_TRANSFORMATION(InvertTransformation, InvertTransformationInstance)
        
    }
}
