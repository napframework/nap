//
//  Grid.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 11/07/2018.
//
//

#ifndef Grid_hpp
#define Grid_hpp

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <glm/vec3.hpp>


#include "Speaker.hpp"
#include "Shape.hpp"

#include "StaticShape.hpp"

#include "Amplitudes.hpp"

namespace casipan {

    class Grid {
        
    public:
        Grid(std::string name) : name(name) {}
        
        // Attempts to add a shape with type T and speakers with the ids 'ids'. Returns true if succeeded, false if the speakers are not valid for this type shape.
        template<typename T> bool addShape(const std::vector<Speaker*>& speakers){

            if(T::isValid(speakers)){
                T shape = T(speakers);
                shapes.push_back(std::make_unique<T>(shape));
                std::cout << "Added shape." << std::endl;
                
                for(auto* speaker : speakers)
                    activeChannels.insert(speaker->channel);
                
                return true;
            }
            else{
                return false;
            }
            
        }
        
        // TODO amps as parameter. Apply grid settings as separate function.
        std::vector<float> getAmplitudesForSource(const Source& source, const GridSettings& settings){
            
            Amplitudes amps(MAX_CHANNELS);
            
            for(auto& shape : shapes)
                shape->getAllAmplitudesForSource(source,settings,amps);
            
            std::vector<float> ampsVector = amps.getAmps();
            
            
            // grid settings & volume
            for(int i = 0; i < ampsVector.size(); i++){
                
                float& amp = ampsVector[i];
                
                // constant power
                amp = constantPower(amp);
                
                // curvature
                float power;
                if(settings.curvature < 0)
                    power = 1. + (-settings.curvature) * 4.;
                else
                    power = 1. / (1. + settings.curvature);
                
                amp = pow(amp, power);
                
                // gain level
                amp *= settings.gainLevel;
                
                // boost level
                if(activeChannels.find(i) != activeChannels.end()){
                    if(amp < settings.boostLevel)
                        amp = settings.boostLevel;
                }
                
                
                // distance attenuation
                if(settings.distanceAttenuation){
                    float distance = glm::distance(source.position, settings.projectionPoint);
                    float distanceValue = glm::clamp<float>(distance / settings.distanceAttenuationThreshold, 0., 1.);
                    
                    // same function as Distance Intensity curvature
                    float gainValue = distanceCurve(distanceValue, settings.distanceAttenuationCurvature, false);
                    
                    amp *= gainValue;
                }
                
                // grid volume slider
                amp *= gridVolume;
                
            }
            
            return ampsVector;
        }
        
        
        
        
        std::string name;
        
        
        void setVolume(float volume){
            gridVolume = volume;
        }
        
    private:
        
        float gridVolume = 1.;
        
        std::set<int> activeChannels; // channels used by this grid. These are the channels on which 'boostLevel' has effect.
        
        std::vector<std::unique_ptr<Shape>> shapes;
        
    };

}

#endif /* Grid_hpp */
