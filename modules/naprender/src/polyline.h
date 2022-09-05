/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "mesh.h"
#include <mathutils.h>
#include <lineutils.h>
#include <unordered_map>

namespace nap
{
	// Forward Declares
	class Core;
	class RenderService;

	/**
	 * Common polyline properties.
	 */
	struct PolyLineProperties
	{
		EMemoryUsage	mUsage = EMemoryUsage::Static;			///< Property: 'Usage' If the line is created once or frequently updated.
		glm::vec4		mColor = { 1.0f, 1.0f, 1.0f,1.0f };			///< Property: 'Color' RGBA color of the line
	};


	//////////////////////////////////////////////////////////////////////////
	// PolyLine
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class for all line specific mesh types.
	 * A PolyLine is a mesh that is always drawn using: LINE_STRIP.
	 * It also has a fixed set of (initial) vertex attributes: Position, Color0, UV0 and Normal.
	 */
	class NAPAPI PolyLine : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		PolyLine(nap::Core& core);

		/**
		 * Create the mesh instance and required vertex attributes (P, Cd, N, Uv)
		 * When implementing a derived PolyLine, make sure to call this init first.
		 * @param errorState contains the error if the line could not be created.
		 * @return if the line is created successfully.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return The polygon line mesh
		 */
		virtual MeshInstance& getMeshInstance() override						{ return *mMeshInstance; }

		/**
		 * @return The polygon line mesh
		 */
		virtual const MeshInstance&	getMeshInstance() const override 			{ return *mMeshInstance; }

		/**
		 *	@return The line position vertex data
		 */
		Vec3VertexAttribute& getPositionAttr();
		
		/**
		*	@return The line position vertex data
		*/
		const Vec3VertexAttribute& getPositionAttr() const;

		/**
		 *	@return The line color vertex data
		 */
		Vec4VertexAttribute& getColorAttr();

		/**
		*	@return The line color vertex data
		*/
		const Vec4VertexAttribute& getColorAttr() const;
		
		/**
		 *	@return The line normal vertex data
		 */
		Vec3VertexAttribute& getNormalAttr();

		/**
		*	@return The line normal vertex data
		*/
		const Vec3VertexAttribute& getNormalAttr() const;
		
		/**
		 *	@return The line uv vertex data
		 */
		Vec3VertexAttribute& getUvAttr();

		/**
		 *	@return The line uv vertex data
		 */
		const Vec3VertexAttribute& getUvAttr() const;

		/**
		 * This call does not accurately interpolate the vertex attribute, therefore this 
		 * function works best with equally distributed vertices
		 * @param attr the attribute to get the value for
		 * @param location the location on the line to get the value for, needs to be within the 0-1 range
		 * @param outValue the interpolated output value
		 * @return the interpolated value of an attribute along the line based on the given location
		 */
		template<typename T>
		void getValue(const VertexAttribute<T>& attr, float location, T& outValue) const;

		/**
		 * @return the interpolated (accurate) value of an attribute along the line based on the given location
		 * This method is more accurate but requires a map that contains the distance along the line for every vertex
		 * You can acquire this map by calling the getDistances function
		 * @param distanceMap map that contains the distance of every vertex along the line
		 * @param attr the attribute to get the value for
		 * @param location the location on the line to get the value for, needs to be within the 0-1 range
		 * @param outValue the interpolated output value 
		 */
		template<typename T>
		void getValue(const std::map<float, int>& distanceMap, const VertexAttribute<T>& attr, float location, T& outValue) const;

		/**
		 * Returns the interpolated normal value along the line, where the normal is correctly interpolated (rotated) based on it's location along the line
		 * This method is more accurate but requires a distance map that contains the distance of every vertex along the line
		 * You can acquire this map by calling the getDistances function
		 * @param distanceMap the map that contains the distance of every vertex along the line
		 * @param attr the normal attribute to get the interpolated (rotated) value for
		 * @param location the location on the line to get the value for, needs to be within the 0-1 range
		 * @param outValue the interpolated output normal
		 */
		void getNormal(const std::map<float, int>& distanceMap, const Vec3VertexAttribute& attr, float location, glm::vec3& outValue) const;

		/**
		 * Utility function that retrieves the distance along the line for every vertex
		 * Note that this call is relatively heavy
		 * @return the length of the line
		 * @param outDistances a list of distances that will be populated with the length at every vertex
		 */
		float getDistances(std::map<float, int>& outDistances) const;

		/**
		 * @return if the line is closed or not
		 */
		virtual bool isClosed() const = 0;

		// Properties associated with a line
		PolyLineProperties mLineProperties;

	protected:
		std::unique_ptr<MeshInstance> mMeshInstance;
		RenderService* mRenderService = nullptr;

		/**
		 * Utility function, creates all the default line attributes: Position, Normal, UV0 and Color0.
		 * @param instance the instance to add the attributes to.
		 */
		static void createVertexAttributes(MeshInstance& instance);
	};


	//////////////////////////////////////////////////////////////////////////
	// Line
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Simple line from a to b. 
	 * The uv coordinates are derived from start and end position of the line (x, y coordinates). 
	 * The normals are perpendicular to the direction of the line, ie: cross({0,0,1}, normalize(End-Start)))
	 */
	class NAPAPI Line : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		Line(nap::Core& core);

		/**
		 * Creates the line
		 * @return if the line was successfully created
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return closed or open based on the 'Closed' property.
		 */
		virtual bool isClosed() const override						{ return mClosed; }

		glm::vec3 mStart	= { -0.5f, 0.0f, 0.0f };				///< Property: 'Start' location of the line
		glm::vec3 mEnd		= { 0.5f, 0.0f, 0.0f };					///< Property: 'End' location of the line
		bool mClosed		= false;								///< Property: 'Closed' if the line is considered closed
		int mVertexCount	= 2;									///< Property: 'Vertices' number of line vertices, defaults to two

	};


	//////////////////////////////////////////////////////////////////////////
	// Rectangle
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Simple rectangle as a polygon line.
	 * Vertex count starts at the lower left corner and evolves counter-clockwise.
	 * The uv coordinates are normalized based on the longest edge.
	 * The normals are interpolated from the edge vertices and point outwards from the center.
	 */
	class NAPAPI Rectangle : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		Rectangle(nap::Core& core);

		/**
		 * Creates the rectangle
		 * @return if the rectangle was successfully created
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return closed
		 */
		virtual bool isClosed() const override						{ return true; }

		glm::vec2 mDimensions = { 1.0f, 1.0f };						///< Property: 'Dimensions' describes the width and height of the rectangle 
	};


	//////////////////////////////////////////////////////////////////////////
	// Circle
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Simple circle as a polygon line. 
	 * The uv's are normalized 0-1, the normals point outwards from the center.
	 */
	class NAPAPI Circle : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		Circle(nap::Core& core);

		/**
		 * Creates the rectangle
		 * @return if the rectangle was successfully created
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return closed
		 */
		virtual bool isClosed() const override					{ return true; }

		float mRadius = 1.0f;									///< Property: 'Radius' circle radius
		int mSegments = 100;									///< Property: 'Segments' number of vertices 
	};


	//////////////////////////////////////////////////////////////////////////
	// Hexagon
	//////////////////////////////////////////////////////////////////////////
	
	/**
	 * Simple hexagon as a polygon line.
	 * The uv's are normalized 0-1, the normals point outwards from the center.
	 */
	class NAPAPI Hexagon : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		Hexagon(nap::Core& core);
		float mRadius = 1.0f;									///< Property: 'Radius' hexagon radius

		/**
		 * Creates the hexagon
		 * @return if the hexagon was successfully created
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return closed
		 */
		virtual bool isClosed() const override					{ return true; }
	};


	//////////////////////////////////////////////////////////////////////////
	// TriangleLine
	//////////////////////////////////////////////////////////////////////////
	
	/**
	 * Simple equal sided triangle.
	 * The uv's are normalized 0-1, the normals point outwards from the center.
	 */
	class NAPAPI TriangleLine : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		TriangleLine(nap::Core& core);
		float mRadius = 1.0f;									///< Property: 'Radius' triangle radius

		/**
		 * Creates the equal sided triangle
		 * @return if the triangle mesh was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return closed
		 */
		virtual bool isClosed() const override					{ return true; }
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////
	
	template<typename T>
	void nap::PolyLine::getValue(const VertexAttribute<T>& attr, float location, T& outValue) const
	{
		return math::getValueAlongLine(attr.getData(), location, isClosed(), outValue);
	}

	template<typename T>
	void nap::PolyLine::getValue(const std::map<float, int>& distanceMap, const VertexAttribute<T>& attr, float location, T& outValue) const
	{
		return math::getValueAlongLine(distanceMap, attr.getData(), location, outValue);
	}
}