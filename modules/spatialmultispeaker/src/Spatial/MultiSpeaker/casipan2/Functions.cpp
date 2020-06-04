//
//  Functions.cpp
//  4dengine
//
//  Created by Casimir Geelhoed on 25/07/2018.
//
//

#include "Functions.hpp"

#include <math.h>
#include <iostream>


namespace casipan {

    glm::vec2 to2D(const glm::vec3& in)
	{
        return glm::vec2(in.x, in.z);
    }

    std::vector<glm::vec2> to2DVec(const std::vector<glm::vec3>& in){
        std::vector<glm::vec2> out;
        out.reserve(in.size());
        for(auto& vec : in){
            out.emplace_back(to2D(vec));
        }
        return out;
    }


    std::vector<glm::vec2> projectAndClip3DPolygonTo2DPolygon(const std::vector<glm::vec3>& polygon3D, const std::vector<glm::vec2>& polygon2D, const glm::vec3& outwardsPlaneNormal, const glm::vec3& planeBasePosition, const glm::vec3& xAxisDirection, const glm::vec3& yAxisDirection, const glm::vec3& projectionDirection, const glm::vec3& projectionPoint, bool orthogonalProjection){
        
        std::vector<glm::vec2> outPoly;

        // clip polygon to plane in 3D space.
        // TODO Optimization: prevent unnecessary allocation?
        const std::vector<glm::vec3> poly = clipPolygonToPlane(polygon3D, planeBasePosition, outwardsPlaneNormal);
        
        if(poly.size() > 0){
            
            // project to plane and convert to 2D coordinates
            for(auto& vertex : poly){
                
                glm::vec3 projectionPosition;
                
                if(orthogonalProjection)
                    projectionPosition = projectPointOnPlane(vertex, projectionDirection, planeBasePosition, outwardsPlaneNormal);
                else
                    projectionPosition = intersectLinePlane(vertex, projectionPoint, planeBasePosition, outwardsPlaneNormal);
                
                glm::vec2 position2D = toPlaneCoordinates(projectionPosition, planeBasePosition, xAxisDirection, yAxisDirection);
                outPoly.push_back(position2D);
            }
            
            // next, in 2D 'plane'-space, clip the projected polygon to the plane.
            if(outPoly.size() > 0){
                outPoly = clipPolygonToPolygon2D(outPoly, polygon2D);
            }
            
        }
        
        return outPoly;
        
    }

    glm::vec2 toPlaneCoordinates(const glm::vec3& position, const glm::vec3& planeBasePosition, const glm::vec3& xAxis, const glm::vec3& yAxis) {
        return glm::vec2(glm::dot(position - planeBasePosition, xAxis),glm::dot(position - planeBasePosition, yAxis));
    }




    // returns whether p1-p2 and p3-p4 intersect each other.
    bool segmentsIntersect(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4) {
        
        // Store the values for fast access and easy
        // equations-to-code conversion
        float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
        float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
        
        float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        // If d is zero, there is no intersection
        if (d == 0) return false;
        
        // Get the x and y
        float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
        float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
        float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
        
        // Check if the x and y coordinates are within both lines
        if ( x < std::min(x1, x2) - SMALL_VALUE || x > std::max(x1, x2) + SMALL_VALUE ||
            x < std::min(x3, x4) - SMALL_VALUE || x > std::max(x3, x4) + SMALL_VALUE) return false;
        if ( y < std::min(y1, y2) - SMALL_VALUE || y > std::max(y1, y2) + SMALL_VALUE ||
            y < std::min(y3, y4) - SMALL_VALUE || y > std::max(y3, y4) + SMALL_VALUE ) return false;
        
        // Return the point of intersection
        return true;
        
    }


    // returns the point of intersection on p1-p2 and p3-p4.
    // (note: could be outside the range. check bool segmentsIntersect() before to check if they actually do intersect.
    glm::vec2 segmentIntersection(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4) {
        // Store the values for fast access and easy
        // equations-to-code conversion
        float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
        float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
        
        float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        // If d is zero, there is no intersection
        if (d == 0) return glm::vec2(0,0);
        
        // Get the x and y
        float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
        float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
        float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
        
        // Check if the x and y coordinates are within both lines
        /*if ( x < std::min(x1, x2) || x > std::max(x1, x2) ||
         x < std::min(x3, x4) || x > std::max(x3, x4) ) return glm::vec2(0,0);
         if ( y < std::min(y1, y2) || y > std::max(y1, y2) ||
         y < std::min(y3, y4) || y > std::max(y3, y4) ) return glm::vec2(0,0);*/
        
        // Return the point of intersection
        return glm::vec2(x,y);
    }


    // TODO bool return type. glm::intersectRayPlane return value meenemen
    glm::vec3 intersectLinePlane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& planeBase, const glm::vec3& outwardsPlaneNormal){
        
        glm::vec3 direction = glm::normalize(p2 - p1);
        return projectPointOnPlane(p1, direction, planeBase, outwardsPlaneNormal);

        
    }


    glm::vec3 projectPointOnPlane(const glm::vec3& point, const glm::vec3& direction, const glm::vec3& planeBase, const glm::vec3& outwardsPlaneNormal){
        
        glm::vec3 dir = glm::normalize(direction);
        
        float intersectionDistance;
        glm::intersectRayPlane(point, dir, planeBase, outwardsPlaneNormal, intersectionDistance);
        glm::vec3 projectionPosition = point + intersectionDistance*dir;
        return projectionPosition;
        
    }


    std::vector<glm::vec3> clipPolygonToPlane(const std::vector<glm::vec3>& vertices, const glm::vec3& planeBase, const glm::vec3& outwardsPlaneNormal){
        
        std::vector<glm::vec3> outVertices;
        outVertices.reserve(vertices.size() * 2); // reserve enough space ahead to optimize.
        
        for(int i = 0; i < vertices.size(); i++){
            
            const glm::vec3& firstPoint = vertices[i];
            const glm::vec3& secondPoint = vertices[(i + 1) % vertices.size()];
            
            bool firstPointIsOutside = glm::dot(firstPoint - planeBase, outwardsPlaneNormal) > 0.;
            bool secondPointIsOutside = glm::dot(secondPoint - planeBase, outwardsPlaneNormal) > 0.;
            
            if(!firstPointIsOutside && !secondPointIsOutside){
                // both behind. skip
            }
            else if(firstPointIsOutside && secondPointIsOutside){
                // both in front. emit first vertex
                outVertices.push_back(firstPoint);
            }
            else if(firstPointIsOutside && !secondPointIsOutside){
                outVertices.push_back(firstPoint);
                outVertices.push_back(intersectLinePlane(firstPoint, secondPoint, planeBase, outwardsPlaneNormal));
            }
            else if (!firstPointIsOutside && secondPointIsOutside){
                outVertices.push_back(intersectLinePlane(secondPoint, firstPoint, planeBase, outwardsPlaneNormal));
            }
        }
        
        return outVertices;

    }


    std::vector<glm::vec2> clipPolygonToPolygon2D(const std::vector<glm::vec2>& subjectPolygon, const std::vector<glm::vec2>& clipPolygon){
        
        bool clockwise = isClockwise(clipPolygon);
        
        std::vector<glm::vec2> outputList = subjectPolygon;
        std::vector<glm::vec2> inputList;
        inputList.reserve(outputList.size());

        for(int i = 0; i < clipPolygon.size(); i++){
            
            const glm::vec2& clipEdgeStart = clipPolygon[i];
            const glm::vec2& clipEdgeEnd = clipPolygon[(i + 1) % clipPolygon.size()];
            
            inputList = outputList;
            outputList.clear();
            
            glm::vec2 s = inputList.back();
            for (glm::vec2& e : inputList){
                if(isInsideEdge(e, clipEdgeStart, clipEdgeEnd, clockwise)){
                    if(!isInsideEdge(s,clipEdgeStart, clipEdgeEnd, clockwise)){
                        outputList.push_back(segmentIntersection(s,e,clipEdgeStart,clipEdgeEnd));
                    }
                    outputList.push_back(e);
                }
                else if (isInsideEdge(s, clipEdgeStart, clipEdgeEnd, clockwise)){
                    outputList.push_back(segmentIntersection(s,e,clipEdgeStart,clipEdgeEnd));
                }
                s = e;
            }

            // return empty vector if nothing is left.
            if(outputList.empty())
                break;
        }
        
        return outputList;
        
    }


    bool isInsideEdge(const glm::vec2& point, const glm::vec2& edgeStart, const glm::vec2& edgeEnd, bool clockwise){
        return (isLeftOfLine(point, edgeStart, edgeEnd) == !clockwise);
    }

    // tells if Point c lies on the left side of directed edge a.b
    // 1 if left, -1 if right, 0 if colinear
    bool isLeftOfLine(const glm::vec2& point, const glm::vec2& lineStart, const glm::vec2& lineEnd){
        return (lineEnd.x - lineStart.x)*(point.y - lineStart.y) - (point.x - lineStart.x)*(lineEnd.y - lineStart.y) > 0;
    }


    // see https://stackoverflow.com/a/1165943/1177065
    bool isClockwise(const std::vector<glm::vec2>& polygon){
        
        float sum = 0;
        
        for(int i = 0; i < polygon.size(); i++){
            
            const glm::vec2 & vertex1 = polygon[i];
            const glm::vec2 & vertex2 = polygon[(i + 1) % polygon.size()];
            
            sum += (vertex2.x - vertex1.x) * (vertex2.y + vertex1.y);
            
        }
        
        return sum > 0;
    }



    bool naivePointInPoly(const glm::vec2& point, const std::vector<glm::vec2>& poly){

        int i, j;
        bool c = false;
        for (i = 0, j = poly.size()-1; i < poly.size(); j = i++) {
            if ( ((poly[i].y>point.y) != (poly[j].y>point.y)) &&
                (point.x < (poly[j].x-poly[i].x) * (point.y-poly[i].y) / (poly[j].y-poly[i].y) + poly[i].x) )
                c = !c;
        }
        return c;

    }


    bool pointInPoly(const glm::vec2& point, const std::vector<glm::vec2>& poly){
        
        if(naivePointInPoly(point,poly))
            return true;
        
        float distance;
        getClosestPointOnEdges(point, poly, distance);
        return distance < 0.0001;
        
    }


    /*bool sameSide(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& a, const glm::vec2& b){
        
        glm::vec2 ba = b-a;
        glm::vec2 p1a = p1-a;
        glm::vec2 p2a = p2-a;
        
        glm::vec3 cp1 = glm::cross(glm::vec3(ba.x,ba.y,0), glm::vec3(p1a.x,p1a.y,0));
        glm::vec3 cp2 = glm::cross(glm::vec3(ba.x,ba.y,0), glm::vec3(p2a.x,p2a.y,0));
        
        return glm::dot(cp1, cp2) >= 0;
        
    }*/



    glm::vec2 getClosestPointOnLineSegment(const glm::vec2& point, const glm::vec2& a, const glm::vec2& b){
        // prevent NaN.
        if(glm::distance2(a,b) < 1e-6f)
            return a;
        else
            return glm::closestPointOnLine(point, a, b);
        
    }


    glm::vec2 getClosestPointOnPolygons(const glm::vec2& point, const std::vector<std::vector<glm::vec2>>& polys){
		
        for (auto& poly : polys){
            // if the point falls within the polygon, return point itself.
            if(naivePointInPoly(point, poly)){
                return point;
            }
        }
		
		glm::vec2 minimumPointOnEdge;
        float minimumDistance = std::numeric_limits<float>::infinity();
		
		for (auto& poly : polys){
			float distance;
			const glm::vec2 pointOnEdge = getClosestPointOnEdges(point, poly, distance);
			
			if (distance < minimumDistance) {
				minimumDistance = distance;
				minimumPointOnEdge = pointOnEdge;
			}
		}
		
		return minimumPointOnEdge;
        
    }


    glm::vec2 getClosestPointOnEdges(const glm::vec2& point, const std::vector<std::pair<glm::vec2,glm::vec2>>& edges){
        float dummy;
        return getClosestPointOnEdges(point, edges, dummy);
    }


    glm::vec2 getClosestPointOnEdges(const glm::vec2& point, const std::vector<std::pair<glm::vec2,glm::vec2>>& edges, float& distance){
		
        glm::vec2 minimumPointOnEdge;
        float minimumDistance_squared = std::numeric_limits<float>::infinity();
		
        for(int i = 0; i < edges.size(); i++){
        	const glm::vec2 pointOnEdge = getClosestPointOnLineSegment(point, edges[i].first, edges[i].second);
        	const float distance_squared = glm::distance2(pointOnEdge,point);
            if (distance_squared < minimumDistance_squared ){
                minimumDistance_squared = distance_squared;
                minimumPointOnEdge = pointOnEdge;
            }
        }
        
        distance = sqrtf(minimumDistance_squared);
        return minimumPointOnEdge;
        
    }


    glm::vec2 getClosestPointOnEdges(const glm::vec2& point, const std::vector<glm::vec2>& polygon){
        float dummy;
        return getClosestPointOnEdges(point, polygon, dummy);
    }



    glm::vec2 getClosestPointOnEdges(const glm::vec2& point, const std::vector<glm::vec2>& polygon, float& distance){
		
        glm::vec2 minimumPointOnEdge;
        float minimumDistance_squared = std::numeric_limits<float>::infinity();
		
        for (int i = 0; i < polygon.size(); i++){
			const glm::vec2 pointOnEdge = getClosestPointOnLineSegment(point, polygon[i], polygon[(i + 1) % polygon.size()]);
			const float distance_squared = glm::distance2(pointOnEdge, point);
            if (distance_squared < minimumDistance_squared){
                minimumDistance_squared = distance_squared;
                minimumPointOnEdge = pointOnEdge;
            }
        }
        
        distance = sqrtf(minimumDistance_squared);
        return minimumPointOnEdge;
        
    }

    float constantPower(float v){
        if(v < SMALL_VALUE)
            return 0.f;
        return sinf((float(M_PI)*v)*0.5f); // normal constant power panning
    }


    double triangleArea(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c){
        const double dArea =
        	(
        		(b.x - a.x) * (c.y - a.y) -
        		(c.x - a.x) * (b.y - a.y)
			) / 2.0;
        return (dArea > 0.0) ? dArea : -dArea;
    }


    glm::vec2 mapQuadrilateralPointToRectanglePoint(const glm::vec2& point, const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3){
        
        const float dU0 = glm::distance(point, getClosestPointOnLineSegment(point, p0, p3));
        const float dV0 = glm::distance(point, getClosestPointOnLineSegment(point, p0, p1));
        const float dU1 = glm::distance(point, getClosestPointOnLineSegment(point, p1, p2));
        const float dV1 = glm::distance(point, getClosestPointOnLineSegment(point, p3, p2));

        const float u = dU0 / (dU0 + dU1);
        const float v = dV0 / (dV0 + dV1);

        return glm::vec2(u, v);
        
    }

    float distanceScale(float value, float inputMin, float inputMax, float outputMin, float outputMax){
        
        if(inputMax - inputMin == 0)
            return 0;
        
        float proportion = (value - inputMin) / (inputMax - inputMin);
        
        return outputMin + (proportion * (outputMax - outputMin));
        
    }

    float distanceCurve(float input, float curvature, bool up){
        if(up){
            if(curvature < 0)
                return std::powf(input, distanceScale(curvature, -1, 0, 10, 1));
            else
                return std::powf(input, distanceScale(curvature, 0, 1, 1, 0.1));
        }
        else{
            if(curvature < 0)
                return std::powf(1. - input, distanceScale(curvature, -1, 0, 10, 1));
            else
                return std::powf(1. - input, distanceScale(curvature, 0, 1, 1, 0.1));
        }
    }

}
