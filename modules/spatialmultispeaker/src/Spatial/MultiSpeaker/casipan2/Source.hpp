//
//  Source.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 11/07/2018.
//
//

#ifndef Source_hpp
#define Source_hpp

#include <array>
#include <vector>
#include <stdio.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext.hpp>

#include "Functions.hpp"

namespace casipan {

    struct GridSettings {
        float gainLevel = 0.;
        float boostLevel = 0.;
        float curvature = 0.;
        glm::vec3 projectionPoint = glm::vec3(0.);
        bool orthogonalProjection = false;
        bool distanceAttenuation = false;
        float distanceAttenuationThreshold = 0.;
        float distanceAttenuationCurvature = 0.;
    };


    class Source {

    public:
        
        Source(const glm::vec3& position, const glm::vec3& dimensions, const glm::vec4& rotation, const std::vector<GridSettings>& gridSettings) : position(position), dimensions(dimensions), rotation(rotation), gridSettings(gridSettings) {
        
            calculateAllData();
        }
        
        glm::vec3 position;
        glm::vec3 dimensions;
        glm::vec4 rotation;
		
        glm::mat4 xform;
        glm::mat4 xformInverse;
        
        std::vector<GridSettings> gridSettings; ///< Grid settings per grid.
        
        bool isPointSource() const {
            return dimensions.x == 0 && dimensions.y == 0 && dimensions.z == 0;
        }
        

        void calculateAllData();
        void calculateTransform();
        void calculateVertices();
        void calculateEdges();
        void calculateTriangles();
        
        const std::array<glm::vec3, 8>& getVertices() const { return vertices; }
        const std::array<glm::vec2, 8>& get2DVertices() const { return vertices2D; }
        const std::vector<std::pair<glm::vec3, glm::vec3>>& getEdges() const { return edges; }
        const std::vector<std::pair<glm::vec2, glm::vec2>>& get2DEdges() const { return edges2D; }
        const std::vector<std::vector<glm::vec3>>& getTriangles() const { return triangles; }
        const std::vector<std::vector<glm::vec2>>& get2DTriangles() const { return triangles2D; }
        
        glm::vec3 getClosestPointTo(const glm::vec3& point) const;
        glm::vec3 getClosestPointTo(const glm::vec3& point, float& distance) const;
        
        glm::vec2 getClosestPointTo2D(const glm::vec2& point) const;
        
    private:
		
        std::array<glm::vec3, 8> vertices;
        std::array<glm::vec2, 8> vertices2D;
        std::vector<std::pair<glm::vec3, glm::vec3>> edges;
        std::vector<std::pair<glm::vec2, glm::vec2>> edges2D;
        std::vector<std::vector<glm::vec3>> triangles;
        std::vector<std::vector<glm::vec2>> triangles2D;
        
    };

}

#endif /* Source_hpp */
