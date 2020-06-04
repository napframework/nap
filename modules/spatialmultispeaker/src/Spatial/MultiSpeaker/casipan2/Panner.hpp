//
//  Panner.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 10/07/2018.
//
//

#ifndef Panner_hpp
#define Panner_hpp

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <ctime>

#include "Speaker.hpp"
#include "Shape.hpp"
#include "Functions.hpp"
#include "Source.hpp"
#include "Grid.hpp"

#include <glm/vec3.hpp>


namespace casipan {

    using glm::vec3;


    class Panner {
        
    public:
        Panner() {}
        
        void addGrid(const std::string& name){
            grids.push_back(std::make_unique<Grid>(name));
            std::cout << "Added grid: " << name << std::endl;

        }
        
        bool addSpeaker(const std::string& name, int channel, const glm::vec3& position, const std::string & speakerType){
            
            //  check double names
            if(getSpeakerByName(name) != nullptr){
                std::cout << "A speaker with name " << name << " already exists." << std::endl;
                return false;
            }
            
            // check double channels
            if(getSpeakerByChannel(channel) != nullptr){
                std::cout << "A speaker with channel " << channel << " already exists." << std::endl;
                return false;
            }
            
            auto speakerPtr = std::make_unique<Speaker>(channel, name, speakerType, position);
            speakers.push_back(std::move(speakerPtr));
            
            if(channel > maxChannel)
                maxChannel = channel;
            
            std::cout << "Added speaker: " << name << std::endl;
            return true;
        }
        

        template<typename T> bool addShapeToGrid(const std::vector<std::string>& speakerNames, const std::string& gridName){
            
            std::vector<Speaker*> speakers; // make vector of observing pointers
            for(auto name : speakerNames){
                Speaker* speakerPtr = getSpeakerByName(name);
                if(speakerPtr == nullptr){
                    std::cout << "Invalid speaker name." << std::endl;
                    return false;
                }
                speakers.push_back(speakerPtr);
            }
            
            Grid* gridPtr = getGridByName(gridName);
            if(gridPtr == nullptr){
                std::cout << "Invalid grid name." << std::endl;
                return false;
            }
            
            return gridPtr->addShape<T>(speakers);

        }
        
        
        std::vector<float> pan(const glm::vec3& position, const glm::vec3& dimensions, const glm::vec4& rotation, const std::vector<GridSettings>& gridSettings) const{
            const Source source = Source(position, dimensions, rotation, gridSettings);
            return getAmplitudesForSource(source);
        }
        
        int getChannelCount() const {
            return maxChannel + 1;
        }
        
        int getGridCount() const {
            return grids.size();
        }
        
        int getSpeakerCount() const {
            return speakers.size();
        }
        
        void setGridVolume(int gridId, float volume){
            grids[gridId]->setVolume(volume);
        }
        
        
        // Used in 4D 1.X.
        std::string getSpeakerTypeForIndex(int id){
            return speakers[id]->speakerType;
        }
        
        // Used in 4D 1.X.
        std::string getGridName(int id){
            return grids[id]->name;
        }
        
        // Used in 4D 1.X.
        std::vector<glm::vec3> getSpeakerPositionsByChannel(){
            std::vector<vec3> outputVec(getChannelCount(), vec3(0,0,0));
            
            for(auto& s : speakers)
                outputVec[s->channel] = s->position;
            
            return outputVec;
        }
        
        
    private:
        
        std::vector<float> getAmplitudesForSource(const Source& source) const{
			
		// todo : remove vector intermediaries
		
            std::vector<float> outVector(maxChannel + 1);
            for(int i = 0; i < grids.size(); i++){
                
                std::vector<float> gridResults = grids[i]->getAmplitudesForSource(source, source.gridSettings[i]);
                
                for(int j = 0; j < outVector.size(); j++)
                    outVector[j] += gridResults[j];
            }
            
            return outVector;
            
        }
        
        Grid* getGridByName(const std::string& gridName){
            for(auto& g : grids){
                if(g->name == gridName)
                    return g.get();
            }
            return nullptr;
        }
        
        Speaker* getSpeakerByName(const std::string& speakerName){
            for(auto& s : speakers){
                if(s->name == speakerName)
                    return s.get();
            }
            return nullptr;
        }
        
        Speaker* getSpeakerByChannel(int channel){
            for(auto& s : speakers){
                if(s->channel == channel)
                    return s.get();
            }
            return nullptr;
        }
        
        std::vector<std::unique_ptr<Grid>> grids;
        std::vector<std::unique_ptr<Speaker>> speakers;
        
        int maxChannel = 0; // highest speakerchannel (this determines the size of the output vector).
        
    };
    
}

#endif /* Panner_hpp */

