#include "WallsShape.h"

// Spatial includes
#include <Spatial/Core/SpatialFunctions.h>
#include <Spatial/Core/ParameterComponent.h>


RTTI_DEFINE_CLASS(nap::spatial::WallsShape)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::WallsShapeInstance)
    RTTI_CONSTRUCTOR(nap::spatial::ShapeBase&)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
     
        void WallsShapeInstance::onInit(nap::EntityInstance* entity)
        {
            
            mLeft = getParameterManager().addParameterBool("left/enable", true);
            mLeft->valueChanged.connect([&](bool){ updateVisualization(); });

            mRight = getParameterManager().addParameterBool("right/enable", true);
            mRight->valueChanged.connect([&](bool){ updateVisualization(); });

            mTop = getParameterManager().addParameterBool("above/enable", true);
            mTop->valueChanged.connect([&](bool){ updateVisualization(); });

            mBottom = getParameterManager().addParameterBool("below/enable", true);
            mBottom->valueChanged.connect([&](bool){ updateVisualization(); });

            mFront = getParameterManager().addParameterBool("front/enable", true);
            mFront->valueChanged.connect([&](bool){ updateVisualization(); });

            mBack = getParameterManager().addParameterBool("back/enable", true);
            mBack->valueChanged.connect([&](bool){ updateVisualization(); });
            
            mDistance = getParameterManager().addParameterFloat("separation", 1.0, 0.0, 10.0);
            mDistance->valueChanged.connect([&](bool){ updateVisualization(); });

            mParticleCount = getParameterManager().addParameterInt("particleCount", 1, 1, 1000, true);
            
            updateVisualization();

            
        }
        
        std::vector<SpatialTransform> WallsShapeInstance::calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time)
        {
            
            std::vector<SpatialTransform> particles;
            
            int density = round(sqrt(mParticleCount->mValue / 6.f));
            if(density < 1)
                density = 1;
            
            // update visualisations if necessary
            if(mCurrentDensity != density)
            {
                mCurrentDensity = density;
                updateVisualization();
            }

            
            if(mLeft->mValue)
            {
                for(int y = 0; y < density; y++)
                {
                    for(int x = 0; x < density; x++)
                    {
                        int x_ = (x + y) % density; // offset to get chessboard pattern in the order of particles.
                        
                        float xPosition = (((x_ * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float yPosition = (((y * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float scale = 1. / density;
                        
                        xPosition *= mDistance->mValue;
                        yPosition *= mDistance->mValue;
                        
                        particles.emplace_back(
                                               GlmFunctions::objectSpaceToWorldSpace(glm::vec3(-1. * mDistance->mValue, xPosition, yPosition), soundObjectTransform),
                                               glm::vec3(0., scale * soundObjectTransform.mScale.y, scale * soundObjectTransform.mScale.z),
                                                soundObjectTransform.mRotation
                                               );
                    }
                }
            }
            
            if(mRight->mValue)
            {
                for(int y = 0; y < density; y++)
                {
                    for(int x = 0; x < density; x++)
                    {
                        int x_ = (x + y) % density;
                        
                        float xPosition = (((x_ * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float yPosition = (((y * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float scale = 1. / density;
                        
                        xPosition *= mDistance->mValue;
                        yPosition *= mDistance->mValue;
                        
                        particles.emplace_back(
                                               GlmFunctions::objectSpaceToWorldSpace(glm::vec3(1. * mDistance->mValue, xPosition, yPosition), soundObjectTransform),
                                               glm::vec3(0., scale * soundObjectTransform.mScale.y, scale * soundObjectTransform.mScale.z),
                                               soundObjectTransform.mRotation
                                               );
                    }
                }
            }

            
            if(mBottom->mValue)
            {
                for(int y = 0; y < density; y++)
                {
                    for(int x = 0; x < density; x++)
                    {
                        int x_ = (x + y) % density; // offset to get checkboard pattern in the order of particles.
                        
                        float xPosition = (((x_ * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float yPosition = (((y * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float scale = 1. / density;
                        
                        xPosition *= mDistance->mValue;
                        yPosition *= mDistance->mValue;
                        
                        particles.emplace_back(
                                               GlmFunctions::objectSpaceToWorldSpace(glm::vec3(xPosition, -1. * mDistance->mValue, yPosition), soundObjectTransform),
                                               glm::vec3(scale * soundObjectTransform.mScale.x, 0, scale * soundObjectTransform.mScale.z),
                                               soundObjectTransform.mRotation
                                               );
                    }
                }
            }
            
            if(mTop->mValue)
            {
                for(int y = 0; y < density; y++)
                {
                    for(int x = 0; x < density; x++)
                    {
                        int x_ = (x + y) % density; // offset to get chessboard pattern in the order of particles.
                        
                        float xPosition = (((x_ * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float yPosition = (((y * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float scale = 1. / density;
                        
                        xPosition *= mDistance->mValue;
                        yPosition *= mDistance->mValue;
                        
                        particles.emplace_back(
                                               GlmFunctions::objectSpaceToWorldSpace(glm::vec3(xPosition, 1. * mDistance->mValue, yPosition), soundObjectTransform),
                                               glm::vec3(scale * soundObjectTransform.mScale.x, 0, scale * soundObjectTransform.mScale.z),
                                               soundObjectTransform.mRotation
                                               );
                    }
                }
            }

            
            if(mFront->mValue)
            {
                for(int y = 0; y < density; y++)
                {
                    for(int x = 0; x < density; x++)
                    {
                        int x_ = (x + y) % density; // offset to get checkboard pattern in the order of particles.
                        
                        float xPosition = (((x_ * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float yPosition = (((y * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float scale = 1. / density;
                        
                        xPosition *= mDistance->mValue;
                        yPosition *= mDistance->mValue;
                        
                        particles.emplace_back(
                                               GlmFunctions::objectSpaceToWorldSpace(glm::vec3(xPosition, yPosition, -1. * mDistance->mValue), soundObjectTransform),
                                               glm::vec3(scale * soundObjectTransform.mScale.x, scale * soundObjectTransform.mScale.y, 0),
                                               soundObjectTransform.mRotation
                                               );
                    }
                }
            }
            
            if(mBack->mValue)
            {
                for(int y = 0; y < density; y++)
                {
                    for(int x = 0; x < density; x++)
                    {
                        int x_ = (x + y) % density; // offset to get checkboard pattern in the order of particles.
                        
                        float xPosition = (((x_ * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float yPosition = (((y * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float scale = 1. / density;
                        
                        xPosition *= mDistance->mValue;
                        yPosition *= mDistance->mValue;
                        
                        particles.emplace_back(
                                               GlmFunctions::objectSpaceToWorldSpace(glm::vec3(xPosition, yPosition, 1. * mDistance->mValue), soundObjectTransform),
                                               glm::vec3(scale * soundObjectTransform.mScale.x, scale * soundObjectTransform.mScale.y, 0),
                                               soundObjectTransform.mRotation
                                               );
                    }
                }
            }
            
            return particles;
        }

        
        void WallsShapeInstance::updateVisualization()
        {
            mVisualizationVertices.clear();
            mVisualizationEdges.clear();

            float distance = mDistance->mValue;
            float multiplier = distance * ((mCurrentDensity - 1.) / (float)mCurrentDensity) + (1. / (float)mCurrentDensity);
            // Formula to retrieve the boundary of the most outer particles of a face of the wall. This boundary depends on the density and the 'distance' parameter. The distance parameter 'plodes' the positions of the particles, but doesn't increase the dimensions of the particles.

            int lastEdge = 0;
            
            // -x plane
            if(mLeft->mValue)
            {
                mVisualizationVertices.push_back(glm::vec3(distance * -.5, multiplier * -.5, multiplier *  -.5));
                mVisualizationVertices.push_back(glm::vec3(distance * -.5, multiplier * -.5, multiplier * .5));
                mVisualizationVertices.push_back(glm::vec3(distance * -.5 , multiplier * .5, multiplier * -.5));
                mVisualizationVertices.push_back(glm::vec3(distance * -.5, multiplier * .5, multiplier * .5));
                
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 0, lastEdge + 1));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 1, lastEdge + 3));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 3, lastEdge + 2));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 2, lastEdge + 0));
                lastEdge += 4;
                
            }
            
            // x plane
            if(mRight->mValue)
            {
                mVisualizationVertices.push_back(glm::vec3(distance * .5, multiplier * -.5, multiplier * -.5));
                mVisualizationVertices.push_back(glm::vec3(distance * .5, multiplier * -.5, multiplier * .5));
                mVisualizationVertices.push_back(glm::vec3(distance * .5, multiplier * .5, multiplier * -.5));
                mVisualizationVertices.push_back(glm::vec3(distance * .5, multiplier * .5, multiplier * .5));
                
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 0, lastEdge + 1));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 1, lastEdge + 3));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 3, lastEdge + 2));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 2, lastEdge + 0));
                lastEdge += 4;

            }
            
            // -y plane
            if(mBottom->mValue)
            {
                mVisualizationVertices.push_back(glm::vec3(multiplier * -.5, distance * -.5, multiplier * -.5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * -.5, distance * -.5, multiplier * .5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * .5, distance * -.5, multiplier *  -.5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * .5, distance * -.5, multiplier * .5));
                
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 0, lastEdge + 1));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 1, lastEdge + 3));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 3, lastEdge + 2));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 2, lastEdge + 0));
                lastEdge += 4;

            }
            
            // y plane
            if(mTop->mValue)
            {
                mVisualizationVertices.push_back(glm::vec3(multiplier * -.5, distance * .5, multiplier * -.5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * -.5, distance * .5, multiplier * .5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * .5, distance * .5, multiplier * -.5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * .5, distance * .5, multiplier * .5));
                
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 0, lastEdge + 1));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 1, lastEdge + 3));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 3, lastEdge + 2));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 2, lastEdge + 0));
                lastEdge += 4;

            }
            
            // -z plane
            if(mFront->mValue)
            {
                mVisualizationVertices.push_back(glm::vec3(multiplier * -.5, multiplier * -.5, distance * -.5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * .5, multiplier * -.5, distance * -.5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * -.5, multiplier * .5, distance * -.5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * .5, multiplier * .5, distance * -.5));
                
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 0, lastEdge + 1));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 1, lastEdge + 3));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 3, lastEdge + 2));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 2, lastEdge + 0));
                lastEdge += 4;

            }
            
            // z plane
            if(mBack->mValue)
            {
                mVisualizationVertices.push_back(glm::vec3(multiplier * -.5, multiplier * -.5, distance * .5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * .5, multiplier * -.5, distance * .5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * -.5, multiplier * .5, distance * .5));
                mVisualizationVertices.push_back(glm::vec3(multiplier * .5, multiplier * .5, distance * .5));
                
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 0, lastEdge + 1));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 1, lastEdge + 3));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 3, lastEdge + 2));
                mVisualizationEdges.push_back(std::pair<int,int>(lastEdge + 2, lastEdge + 0));
                lastEdge += 4;

            }
            
        }

    }
    
}
