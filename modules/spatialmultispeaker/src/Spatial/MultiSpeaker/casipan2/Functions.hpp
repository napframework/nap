//
//  Functions.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 25/07/2018.
//
//

#ifndef Functions_hpp
#define Functions_hpp

#include <stdio.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <algorithm>
#include <glm/ext.hpp>
#include <vector>
#include <utility/dllexport.h>


#define SMALL_VALUE 0.00001

namespace casipan {

    glm::vec2 NAPAPI to2D(const glm::vec3& in);
    std::vector<glm::vec2> NAPAPI to2DVec(const std::vector<glm::vec3>& in);

    template<class T>
    const T& clamp(const T& x, const T& lower, const T& upper) {
        return std::min(upper, std::max(x, lower));
    }

    // returns the closest point to 'point' on a set of 2D polygons. Used in projection panning. Should be safe and protected against edge case errors.
    glm::vec2 NAPAPI getClosestPointOnPolygons(const glm::vec2& point, const std::vector<std::vector<glm::vec2>>& polys);

    // returns the closest point to 'point' on a set of 2D lines.
    glm::vec2 NAPAPI getClosestPointOnEdges(const glm::vec2& point, const std::vector<std::pair<glm::vec2,glm::vec2>>& edges);
    glm::vec2 NAPAPI getClosestPointOnEdges(const glm::vec2& point, const std::vector<std::pair<glm::vec2,glm::vec2>>& edges, float& distance);

    // returns the closest point to 'point' on the 2D lines of a polygon
    glm::vec2 NAPAPI getClosestPointOnEdges(const glm::vec2& point, const std::vector<glm::vec2>& polygon);
    glm::vec2 NAPAPI getClosestPointOnEdges(const glm::vec2& point, const std::vector<glm::vec2>& polygon, float& distance);


    // returns the closest point to 'point' on a line segment.
    glm::vec2 NAPAPI getClosestPointOnLineSegment(const glm::vec2& point, const glm::vec2& a, const glm::vec2& b);


    // projects and clips a 3D polygon to a 2D polygon on a 2D plane. polygon2D and the returned polygons are in 'plane coordinates', where (0,0) corresponds with planeBasePosition
    // First clips the 3D polygon on the plane, then projects to 2D. Then in 2D space clips the polygon onto polygon2D.
    std::vector<glm::vec2> NAPAPI projectAndClip3DPolygonTo2DPolygon(const std::vector<glm::vec3>& polygon3D, const std::vector<glm::vec2>& polygon2D, const glm::vec3& outwardsPlaneNormal, const glm::vec3& planeBasePosition, const glm::vec3& xAxisDirection, const glm::vec3& yAxisDirection, const glm::vec3& projectionDirection, const glm::vec3& projectionPoint, bool orthogonalProjection);

    // helper function for projectAndClip3DPolygonsTo2DPolygons that converts a 3D projected position to 2D coordinates in the given axes system.
    glm::vec2 NAPAPI toPlaneCoordinates(const glm::vec3& position, const glm::vec3& planeBasePosition, const glm::vec3& xAxis, const glm::vec3& yAxis);

    bool NAPAPI segmentsIntersect(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4);

    glm::vec2 NAPAPI segmentIntersection(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec2& p4);


    glm::vec3 NAPAPI intersectLinePlane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& planeBase, const glm::vec3& planeNormal);


    glm::vec3 NAPAPI projectPointOnPlane(const glm::vec3& point, const glm::vec3& direction, const glm::vec3& planeBase, const glm::vec3& planeNormal);


    std::vector<glm::vec3> NAPAPI clipPolygonToPlane(const std::vector<glm::vec3>& vertices, const glm::vec3& planeBase, const glm::vec3& outwardsPlaneNormal);
    //std::vector<glm::vec3> clipTriangleToPlane(Triangle triangle, glm::vec3 planeBase, glm::vec3 outwardsPlaneNormal);


    // clips a 2D polygon to another 2D polygon using the Sutherland-Hodgman algorithm
    std::vector<glm::vec2> NAPAPI clipPolygonToPolygon2D(const std::vector<glm::vec2>& subjectPolygon, const std::vector<glm::vec2>& clipPolygon);

    // helper function for clipPolyToPoly2D. clockwise indicates whether the edge is going in clockwise or counterclockwise direction.
    bool NAPAPI isInsideEdge(const glm::vec2& point, const glm::vec2& edgeStart, const glm::vec2& edgeEnd, bool clockwise);

    bool NAPAPI isLeftOfLine(const glm::vec2& point, const glm::vec2& lineStart, const glm::vec2& lineEnd);

    // helper function for clipPolyToPoly2D. Determines if a polygon is listed in clockwise or counterclockwise direction.
    bool NAPAPI isClockwise(const std::vector<glm::vec2>& polygon);

    // no protection against edge case errors. Check distance to boundaries afterwards.
    //(this function is redundant now we have naivePointInPoly)
    //bool naivePointInTriangle(glm::vec2 p, glm::vec2 a, glm::vec2 b, glm::vec2 c);

    // no protection against edge case errors. Check distance to boundaries afterwards.
    bool NAPAPI naivePointInPoly(const glm::vec2& point, const std::vector<glm::vec2>& poly);

    // includes edge checking
    bool NAPAPI pointInPoly(const glm::vec2& point, const std::vector<glm::vec2>& poly);

    //bool sameSide(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& a, const glm::vec2& b);

    float NAPAPI constantPower(float v);

    double NAPAPI triangleArea(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c);

    // see: https://math.stackexchange.com/a/104595/454427
    glm::vec2 NAPAPI mapQuadrilateralPointToRectanglePoint(const glm::vec2& point, const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3);


    // below functions are copied from 2.0 source. Necessary to keep 4dpan independent from framework/version.

    /**
     * Helper function for the distanceCurve() function.
     */
    float NAPAPI distanceScale(float value, float inputMin, float inputMax, float outputMin, float outputMax);

    /**
     * Function that implements the 1.X 'distance' curve with adjustable curvature.
     * @param input: the input value between 0 and 1.
     * @param curvature: the '4D' curvature value between -1 & 1. 0 is linear, >0 is convex, <0 is concave.
     * @param up: whether the curve goes from 0 to 1 or from 1 to 0.
     * @return: the curved value between 0 and 1.
     */
    float NAPAPI distanceCurve(float input, float curvature, bool up);


}

#endif /* Functions_hpp */
