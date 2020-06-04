//
//  Source.cpp
//  4dengine
//
//  Created by Casimir Geelhoed on 11/07/2018.
//
//

#include "Source.hpp"

#include <iterator>


namespace casipan {

    using namespace glm;

    glm::vec3 Source::getClosestPointTo(const glm::vec3& point, float& distance) const {
        
        // optimization: if point source just output the position.
        if(isPointSource())
            return position;
        
        // transform point to src coordinates
        const vec4 px = xformInverse * vec4(point.x, point.y, point.z, 1.f);
        
        vec3 closest = vec3(px.x, px.y, px.z);
        const vec3 halfDim = dimensions * 0.5f;
        closest = max(closest, halfDim * -1.0f);
        closest = min(closest, halfDim);
        
        // transform point back to world coordinates
        const vec4 tempResult = xform * vec4(closest.x, closest.y, closest.z, 1.f);
        
        vec3 result = vec3(tempResult.x, tempResult.y, tempResult.z);
        
        // round to speaker
        if(glm::distance(result,point) < 0.0001)
            result = point;
        
        distance = glm::distance(result,point);
        
        return result;
        
    }


    glm::vec3 Source::getClosestPointTo(const glm::vec3& point) const {
        float dummy;
        return getClosestPointTo(point, dummy);
    }



    glm::vec2 Source::getClosestPointTo2D(const glm::vec2& point) const {
        
        // if this source is a point source, just return its position.
        if(isPointSource())
            return to2D(position);
        
        
        return getClosestPointOnPolygons(point, get2DTriangles());

    }


    void Source::calculateAllData(){
		
        calculateTransform();
        calculateVertices();
        calculateEdges();
        calculateTriangles();
        
    }


	void Source::calculateTransform() {
		const vec3 axis = normalize(vec3(rotation[1], rotation[2], rotation[3]));
        const float angle = rotation[0];
        xform = translate(mat4(), position);
        xform = rotate(xform, angle, axis);
        xformInverse = inverse(xform);
	}


    void Source::calculateVertices() {
        
        const glm::vec3 halfDim = dimensions * 0.5f;
        
        const glm::vec3 unrotatedVertices[8] =
        {
        	glm::vec3(- halfDim.x, - halfDim.y, - halfDim.z),
        	glm::vec3(- halfDim.x, - halfDim.y,   halfDim.z),
        	glm::vec3(- halfDim.x,   halfDim.y, - halfDim.z),
        	glm::vec3(- halfDim.x,   halfDim.y,   halfDim.z),
        	glm::vec3(  halfDim.x, - halfDim.y, - halfDim.z),
        	glm::vec3(  halfDim.x, - halfDim.y,   halfDim.z),
        	glm::vec3(  halfDim.x,   halfDim.y, - halfDim.z),
        	glm::vec3(  halfDim.x,   halfDim.y,   halfDim.z)
        };
		
        //rotate vertices
        const glm::vec3 axis = glm::vec3(rotation[1], rotation[2], rotation[3]);
        const float angle = rotation[0];
		
		int index = 0;
        for(const vec3& v : unrotatedVertices){
            vertices[index++] = glm::rotate(v, angle, axis) + position;
        }
        
        // 2D vertices
        index = 0;
        for(auto& vertex : vertices)
            vertices2D[index++] = to2D(vertex);
        
    }



    // should be called after calculateVertices().
    void Source::calculateEdges() {
        
        edges.clear();
		
        edges.reserve(12);
		
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[0],vertices[1]));
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[0],vertices[2]));
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[2],vertices[3]));
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[1],vertices[3]));
        
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[4],vertices[5]));
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[4],vertices[6]));
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[6],vertices[7]));
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[5],vertices[7]));
        
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[0],vertices[4]));
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[1],vertices[5]));
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[2],vertices[6]));
        edges.push_back(std::pair<glm::vec3, glm::vec3>(vertices[3],vertices[7]));
        
        edges2D.clear();
		
        edges2D.reserve(12);
        
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[0],vertices2D[1]));
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[0],vertices2D[2]));
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[2],vertices2D[3]));
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[1],vertices2D[3]));
        
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[4],vertices2D[5]));
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[4],vertices2D[6]));
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[6],vertices2D[7]));
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[5],vertices2D[7]));
        
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[0],vertices2D[4]));
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[1],vertices2D[5]));
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[2],vertices2D[6]));
        edges2D.push_back(std::pair<glm::vec2, glm::vec2>(vertices2D[3],vertices2D[7]));
        
    }


    // should be called after calculateVertices()
    void Source::calculateTriangles() {
        
        triangles.clear();
		
        triangles.reserve(12);
        
        triangles.push_back(std::vector<glm::vec3>({vertices[0], vertices[1], vertices[2]}));
        triangles.push_back(std::vector<glm::vec3>({vertices[1], vertices[2], vertices[3]}));
        
        triangles.push_back(std::vector<glm::vec3>({vertices[0], vertices[4], vertices[6]}));
        triangles.push_back(std::vector<glm::vec3>({vertices[0], vertices[6], vertices[2]}));
        
        triangles.push_back(std::vector<glm::vec3>({vertices[4], vertices[5], vertices[7]}));
        triangles.push_back(std::vector<glm::vec3>({vertices[4], vertices[7], vertices[6]}));
        
        triangles.push_back(std::vector<glm::vec3>({vertices[5], vertices[1], vertices[3]}));
        triangles.push_back(std::vector<glm::vec3>({vertices[5], vertices[3], vertices[7]}));
        
        triangles.push_back(std::vector<glm::vec3>({vertices[6], vertices[7], vertices[3]}));
        triangles.push_back(std::vector<glm::vec3>({vertices[6], vertices[3], vertices[2]}));
        
        triangles.push_back(std::vector<glm::vec3>({vertices[1], vertices[0], vertices[4]}));
        triangles.push_back(std::vector<glm::vec3>({vertices[1], vertices[4], vertices[5]}));
        
        triangles2D.clear();
		
        triangles2D.reserve(12);
        
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[0], vertices2D[1], vertices2D[2]}));
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[1], vertices2D[2], vertices2D[3]}));
        
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[0], vertices2D[4], vertices2D[6]}));
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[0], vertices2D[6], vertices2D[2]}));
        
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[4], vertices2D[5], vertices2D[7]}));
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[4], vertices2D[7], vertices2D[6]}));
        
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[5], vertices2D[1], vertices2D[3]}));
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[5], vertices2D[3], vertices2D[7]}));
        
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[6], vertices2D[7], vertices2D[3]}));
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[6], vertices2D[3], vertices2D[2]}));
        
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[1], vertices2D[0], vertices2D[4]}));
        triangles2D.push_back(std::vector<glm::vec2>({vertices2D[1], vertices2D[4], vertices2D[5]}));
        
    }

}
