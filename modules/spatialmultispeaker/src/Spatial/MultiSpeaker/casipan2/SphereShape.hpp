//
//  SphereShape.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 19/02/2020.
//
//

#ifndef SphereShape_hpp
#define SphereShape_hpp

#include <stdio.h>
#include <vector>
#include "Shape.hpp"

#include <glm/glm.hpp>


namespace casipan {
    
    using glm::vec3;
    

    /**
     *
     * Shape that divides the volume in between the speakers into tetrahedrons of a,b,c,O, d,e,f,O etc, where a,b,c,d,.. are speakers and O is the average position of the speakers (the 'center').
     * Within each tetrahedron, the panning algorithm works like the tetrahedronshape. The value of phantom speaker O is divided over all speakers of the sphere.
     * The speakers are set as a single list A B C D E F G H I etc, where A B C O, D E F O, G H I O form tetrahedrons. It is allowed to have duplicates, so a speaker can be part of multiple tetrahedrons.
     */
    class SphereShape : public Shape {
        
    public:
        
        SphereShape(const std::vector<Speaker*> speakers) : Shape(speakers) {

            assert(speakers.size() % 3 == 0);
            for(int i = 0; i < speakers.size(); i += 3)
            {
                triangles.push_back({speakers[i], speakers[i+1], speakers[i+2]});
                originPosition += speakers[i]->position;
                originPosition += speakers[i+1]->position;
                originPosition += speakers[i+2]->position;
            }
            
            originPosition = originPosition / (float)speakers.size();
            
            for(int i = 0 ; i < triangles.size(); i++)
                tetrahedronVolumes.push_back(getVolume(triangles[i][0]->position, triangles[i][1]->position, triangles[i][2]->position, originPosition));
            
            
            uniqueSpeakers = std::vector<Speaker*>(speakers);
            std::sort(uniqueSpeakers.begin(), uniqueSpeakers.end());
            auto last = std::unique(uniqueSpeakers.begin(), uniqueSpeakers.end());
            uniqueSpeakers.erase(last, uniqueSpeakers.end());
            
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override {
            
            float originValue = 0.;
            
            // set speaker values with amps.set().
            for(int t = 0; t < triangles.size(); t++)
            {
                for(int i = 0; i < 4; i++)
                {
                    float distance;
                    glm::vec3 closestPointToSpeaker;
                    if(i == 3)
                        closestPointToSpeaker = source.getClosestPointTo(originPosition, distance);
                    else
                        closestPointToSpeaker = source.getClosestPointTo(triangles[t][i]->position, distance);
                    
                    if(isInside(closestPointToSpeaker, triangles[t][0]->position, triangles[t][1]->position, triangles[t][2]->position, originPosition)){
                        
                        if(i == 0)
                            amps.set(triangles[t][0]->channel, getVolume(closestPointToSpeaker, triangles[t][1]->position, triangles[t][2]->position, originPosition)/tetrahedronVolumes[t]);
                        else if(i == 1)
                            amps.set(triangles[t][1]->channel, getVolume(closestPointToSpeaker, triangles[t][2]->position, originPosition, triangles[t][0]->position)/tetrahedronVolumes[t]);
                        else if(i == 2)
                            amps.set(triangles[t][2]->channel, getVolume(closestPointToSpeaker, originPosition, triangles[t][0]->position, triangles[t][1]->position)/tetrahedronVolumes[t]);
                        else if(i == 3)
                            originValue = std::max<float>(getVolume(closestPointToSpeaker, triangles[t][0]->position, triangles[t][1]->position, triangles[t][2]->position)/tetrahedronVolumes[t], originValue);
                    }
                }
            }
            
            // distribute originValue (phantom speaker value) over speakers.
            if(originValue > 0)
            {
                float addedValuePerSpeaker = originValue / uniqueSpeakers.size();
                
                for(int i = 0; i < uniqueSpeakers.size(); i++)
                    amps.add(uniqueSpeakers[i]->channel, addedValuePerSpeaker);
            }
        }
        
        static bool isValid(std::vector<Speaker*> speakers){
            return speakers.size() % 3 == 0;
        }
        
            
    private:
        // copied from TetrahedronShape
        bool isInside(glm::vec3 position, glm::vec3 speaker0Position, glm::vec3 speaker1Position, glm::vec3 speaker2Position, glm::vec3 speaker3Position){
            return  SameSide(speaker0Position, speaker1Position, speaker2Position, speaker3Position, position) &&
            SameSide(speaker1Position, speaker2Position, speaker3Position, speaker0Position, position) &&
            SameSide(speaker2Position, speaker3Position, speaker0Position, speaker1Position, position) &&
            SameSide(speaker3Position, speaker0Position, speaker1Position, speaker2Position, position);
        }
        
        // copied from TetrahedronShape
        // https://stackoverflow.com/a/12516553/1177065
        float getVolume(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d){
            return (1./6.) * glm::length(glm::determinant(glm::mat3(a-d,b-d,c-d)));
        }
        
        float epsilon = 0.0001;
        
        // copied from TetrahedronShape
        bool SameSide(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, glm::vec3 p)
        {
            glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
            float dotV4 = glm::dot(normal, v4 - v1);
            float dotP = glm::dot(normal, p - v1);
            return (dotV4 >= -epsilon && dotP >= -epsilon) || (dotV4 <= epsilon && dotP <= epsilon);
        }

        glm::vec3 originPosition;
        std::vector<std::vector<Speaker*>> triangles; // the triangles that, together with the originPosition, make up the tetrahedrons.
        std::vector<float> tetrahedronVolumes; // the volume of each tetrahedron, so it doesn't have to be recalculated every cycle.
        
        std::vector<Speaker*> uniqueSpeakers; // the list of speakers without duplicates.
        
        
    };
    
}

#endif /* SphereShape_hpp */
