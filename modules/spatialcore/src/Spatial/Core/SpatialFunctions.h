#pragma once

// Rtti includes
#include <rtti/rtti.h>

// Glm includes
#include <glm/glm.hpp>


namespace nap
{

    namespace spatial
    {

        // Forward declarations
        class SpatialTransform;


        /**
         * Class that's used to expose glm functions and dependent functions to python.
         */
        class NAPAPI GlmFunctions {
            RTTI_ENABLE()
        public:
            GlmFunctions() = default;
            virtual ~GlmFunctions() { }
            
            /**
             * Returns the distances between a & b.
             */
            static float distance(glm::vec3 a, glm::vec3 b);
            
            /**
             * Rotates the point according to th egiven angle axis rotation.
             */
            static glm::vec3 rotate(glm::vec3 point, glm::vec4 angleAxis);
            
            /**
             * Returns 2D simplex noise value according to the glm::simplex function. 
             */
            static float simplex(float t, float y);
            
            /**
             * Converts XYZ rotation to angle axis rotation.
             */
            static glm::vec4 xyzToAngleAxis(glm::vec3 xyz);
            
            /**
             * Converts degrees to radians.
             */
            static float toRadians(float inDegrees);
            
            /**
             * Returns an angle axis rotation that points from the eye to the subject (used for 'center' rotation mode).
             */
            static glm::vec4 lookAtInAngleAxis(glm::vec3 eye, glm::vec3 subject);
                        
            /**
             * Composites an angle-axis rotation with an angle axis rotation.
             */
            static glm::vec4 compositeAngleAxisRotations(const glm::vec4& rotation, const glm::vec4& angleAxisRotationOffset);

            
            /**
             * Converts 'shape space' position to 'world space' position.
             * 'world space': the real coordinates in meters.
             * 'shape space': the coordinates in meters in relation to the center of the sound object, unrotated.
             * 'object space': the coordinates normalized within [-1,1] in relation to the center of the sound object.
             */
            static glm::vec3 shapeSpaceToWorldSpace(const glm::vec3& shapeSpacePosition, const SpatialTransform& soundObjectTransform);
            
            
            /**
             * Converts 'world space' position to 'shape space' position.
             */
            static glm::vec3 worldSpaceToShapeSpace(const glm::vec3& worldSpacePosition, const SpatialTransform& soundObjectTransform);

            
            /**
             * Converts 'object space' position to 'world space' position.
             */
            static glm::vec3 objectSpaceToWorldSpace(const glm::vec3& objectSpacePosition, const SpatialTransform& soundObjectTransform);
            
            /**
             * Converts 'world space' position to 'object space' position.
             */
            static glm::vec3 worldSpaceToObjectSpace(const glm::vec3& worldSpacePosition, const SpatialTransform& soundObjectTransform);
            
            /**
             * Converts 'object space' position to 'shape space' position (multiplies by half of sound object dimensions)
             */
            static glm::vec3 objectSpaceToShapeSpace(const glm::vec3& objectSpacePosition, const SpatialTransform& soundObjectTransform);

            /**
             * Converts 'shape space' position to 'object space' position (divides by half of sound object dimensions)
             */
            static glm::vec3 shapeSpaceToObjectSpace(const glm::vec3& shapeSpacePosition, const SpatialTransform& soundObjectTransform);

            


            
        };
        
        /**
         * Class for some 1.X math helper functions.
         */
        class NAPAPI Functions {
            
        public:
            Functions() = default;
            
            /**
             * Helper function for the distanceCurve() function.
             */
            static float distanceScale(float value, float inputMin, float inputMax, float outputMin, float outputMax);
            
            /**
             * Function that implements the 1.X 'distance' curve with adjustable curvature.  
             * @param input: the input value between 0 and 1.
             * @param curvature: the '4D' curvature value between -1 & 1. 0 is linear, >0 is convex, <0 is concave.
             * @param up: whether the curve goes from 0 to 1 or from 1 to 0.
             * @return: the curved value between 0 and 1.
             */
            static float distanceCurve(float input, float curvature, bool up);
            
            constexpr static const float LN20K = 9.90348755254f;
            constexpr static const float LN20 = 2.99573227355f;
            
            /**
             * Function that converts a fraction between 0 and 1 to a frequency between 20 and 20000 using logarithmic scaling.
             */
            static float getFrequencyForFraction(float fraction);
            
        };

    }

}
