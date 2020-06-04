//
//  XLineShape.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 19/07/2018.
//
//

#ifndef XLineShape_hpp
#define XLineShape_hpp

#include <stdio.h>

#include "Shape.hpp"

namespace casipan {

    class XLineShape : public Shape {
        
    public:
        
        XLineShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            assert(speakers.size() == 2);
            
            minX = speakers[0]->position.x;
            maxX = speakers[1]->position.x;
            lenX = maxX - minX;

        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {

            
            
            if(source.isPointSource()){
                
                if(source.position.x >= minX && source.position.x <= maxX){
                    
                    float posX = source.position.x - minX;
                    
                    amps.set(speakers[0]->channel, ((lenX - posX) / lenX));
                    amps.set(speakers[1]->channel, posX / lenX);

                }
            }
            else{
                
                // find minimal and maximal x values of source and set amps accordingly.
                
                const auto& vertices = source.getVertices();
                
                float sourceMaxX = vertices[0].x;
                float sourceMinX = vertices[0].x;
                
                float ampA;
                float ampB;
                
                
                for(auto& vertex : vertices){
                    if(vertex.x > sourceMaxX)
                        sourceMaxX = vertex.x;
                    else if(vertex.x < sourceMinX)
                        sourceMinX = vertex.x;
                }
                
                if(sourceMinX <= minX && sourceMaxX >= minX)
                    ampA = 1.0;
                else if(sourceMinX > minX && sourceMinX <= maxX)
                    ampA = 1.0 - (sourceMinX - minX) / (maxX - minX);
                else
                    ampA = 0.0;
                
                if(sourceMaxX >= maxX && sourceMinX <= maxX)
                    ampB = 1.0;
                else if(sourceMaxX < maxX && sourceMaxX >= minX)
                    ampB = (sourceMaxX - minX ) / (maxX - minX);
                else
                    ampB = 0.0;
                
                amps.set(speakers[0]->channel, ampA);
                amps.set(speakers[1]->channel, ampB);

            }
            
        }
            
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 2;
        }
        
    private:
        float minX, maxX, lenX;
            
    };



    class XStartPointShape : public Shape {
        
    public:
        XStartPointShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            assert(speakers.size() == 1);
        }
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            float xValue;
            
            if(source.isPointSource()){
                xValue = source.position.x;
            }
            else{
                
                const auto& vertices = source.getVertices();

                xValue = vertices[0].x;
                
                for(auto& vertex : vertices){
                    if(vertex.x < xValue)
                        xValue = vertex.x;
                }

                
            }

            if(xValue <= speakers[0]->position.x)
                amps.set(speakers[0]->channel, 1.0);

        }

        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 1;
        }
        
    };
        
    class XEndPointShape : public Shape {
    public:
        XEndPointShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            assert(speakers.size() == 1);
        }
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            float xValue;
            
            if(source.isPointSource()){
                xValue = source.position.x;
            }
            else{
                
                const auto& vertices = source.getVertices();
                
                xValue = vertices[0].x;
                
                for(auto& vertex : vertices){
                    if(vertex.x > xValue)
                        xValue = vertex.x;
                }
                
                
            }
            
            if(xValue >= speakers[0]->position.x)
                amps.set(speakers[0]->channel, 1.0);
                
        }
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 1;
        }

    };


    // ---------

    class ZLineShape : public Shape {
        
    public:
        
        ZLineShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            assert(speakers.size() == 2);
            
            minZ = speakers[0]->position.z;
            maxZ = speakers[1]->position.z;
            lenZ = maxZ - minZ;
            
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            
            
            if(source.isPointSource()){
                
                if(source.position.z >= minZ && source.position.z <= maxZ){
                    
                    float posZ = source.position.z - minZ;
                    
                    amps.set(speakers[0]->channel, ((lenZ - posZ) / lenZ));
                    amps.set(speakers[1]->channel, posZ / lenZ);
                    
                }
            }
            else{
                
                // find minimal and maximal z values of source and set amps accordingly.
                
                const auto& vertices = source.getVertices();
                
                float sourceMaxZ = vertices[0].z;
                float sourceMinZ = vertices[0].z;
                
                float ampA;
                float ampB;
                
                
                for(auto& vertex : vertices){
                    if(vertex.z > sourceMaxZ)
                        sourceMaxZ = vertex.z;
                    else if(vertex.z < sourceMinZ)
                        sourceMinZ = vertex.z;
                }
                
                if(sourceMinZ <= minZ && sourceMaxZ >= minZ)
                    ampA = 1.0;
                else if(sourceMinZ > minZ && sourceMinZ <= maxZ)
                    ampA = 1.0 - (sourceMinZ - minZ) / (maxZ - minZ);
                else
                    ampA = 0.0;
                
                if(sourceMaxZ >= maxZ && sourceMinZ <= maxZ)
                    ampB = 1.0;
                else if(sourceMaxZ < maxZ && sourceMaxZ >= minZ)
                    ampB = (sourceMaxZ - minZ ) / (maxZ - minZ);
                else
                    ampB = 0.0;
                
                amps.set(speakers[0]->channel, ampA);
                amps.set(speakers[1]->channel, ampB);
                
            }
            
        }
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 2;
        }
        
    private:
        float minZ, maxZ, lenZ;
        
    };



    class ZStartPointShape : public Shape {
        
    public:
        ZStartPointShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            assert(speakers.size() == 1);
        }
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            float zValue;
            
            if(source.isPointSource()){
                zValue = source.position.z;
            }
            else{
                
                const auto& vertices = source.getVertices();
                
                zValue = vertices[0].z;
                
                for(auto& vertex : vertices){
                    if(vertex.z < zValue)
                        zValue = vertex.z;
                }
                
                
            }
            
            if(zValue <= speakers[0]->position.z)
                amps.set(speakers[0]->channel, 1.0);
                
                }
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 1;
        }
        
    };

    class ZEndPointShape : public Shape {
    public:
        ZEndPointShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            assert(speakers.size() == 1);
        }
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            float zValue;
            
            if(source.isPointSource()){
                zValue = source.position.z;
            }
            else{
                
                const auto& vertices = source.getVertices();
                
                zValue = vertices[0].z;
                
                for(auto& vertex : vertices){
                    if(vertex.z > zValue)
                        zValue = vertex.z;
                }
                
                
            }
            
            if(zValue >= speakers[0]->position.z)
                amps.set(speakers[0]->channel, 1.0);
                
                }
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 1;
        }
        
    };

}





#endif /* XLineShape_hpp */
