#pragma once

#include "mesh.h"
#include <mathutils.h>
#include <lineutils.h>
#include <unordered_map>

namespace nap
{
	/**
	 * Properties commonly associated with a PolyLine
	 */
	struct PolyLineProperties
	{
		glm::vec4 mColor = { 1.0f, 1.0f, 1.0f,1.0f };	// Color of the line
	};


	/**
	 * Base class for all line specific mesh types
	 * A PolyLine is the same as a mesh but limited to LINE_STRIP and LINE_LOOP drawing modes
	 * It also has a fixed set of (initial) vertex attributes: Position, Color, Uv and Normals
	 */
	class NAPAPI PolyLine : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		/**
		 * Create the mesh instance and the necessary vertex attributes (P, Cd, N, Uv)
		 * When implementing a derived PolyLine, make sure to call the base init first
		 * @return if the mesh was and attributes were created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	@return the mesh associated with this poly line
		 */
		virtual MeshInstance& getMeshInstance() override						{ return *mMeshInstance; }

		/**
		 *	@return const reference to the mesh associated with this poly line
		 */
		virtual const MeshInstance&	getMeshInstance() const override 			{ return *mMeshInstance; }

		/**
		 *	@return the position vertex data
		 */
		Vec3VertexAttribute& getPositionAttr();
		
		/**
		*	@return the position vertex data
		*/
		const Vec3VertexAttribute& getPositionAttr() const;

		/**
		 *	@return the color vertex data
		 */
		Vec4VertexAttribute& getColorAttr();

		/**
		*	@return the color vertex data
		*/
		const Vec4VertexAttribute& getColorAttr() const;
		
		/**
		 *	@return the normal data
		 */
		Vec3VertexAttribute& getNormalAttr();

		/**
		*	@return the normal data
		*/
		const Vec3VertexAttribute& getNormalAttr() const;
		
		/**
		 *	@return the uv data
		 */
		Vec3VertexAttribute& getUvAttr();

		/**
		*	@return the uv data
		*/
		const Vec3VertexAttribute& getUvAttr() const;

		/**
		 * This call does not accurately interpolate the vertex attribute, therefore this 
		 * function works best with equally distributed vertices
		 * @param attr the attribute to get the value for
		 * @param location the location on the line to get the value for, needs to be within the 0-1 range
		 * @param outValue the interpolated output value
		 * @return the interpolated value of an attribute along the line based on @location
		 */
		template<typename T>
		void getValue(const VertexAttribute<T>& attr, float location, T& outValue) const;

		/**
		 * @return the interpolated (accurate) value of an attribute along the line based on @location
		 * This method is more accurate but requires a map that contains the distance along the line for every vertex
		 * You can acquire this map by calling the @getDistances function
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
		 * You can acquire this map by calling the @getDistances function
		 * @param distanceMap the map that containst the distance of every vertex along the line
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
		*	@return if the line is closed or not
		*/
		bool isClosed() const;

		// Properties associated with a line
		PolyLineProperties mLineProperties;

	protected:
		std::unique_ptr<MeshInstance> mMeshInstance;

		// Creates the default vertex line attributes, useful for easy access
		static void createVertexAttributes(MeshInstance& instance);
	};

	//////////////////////////////////////////////////////////////////////////


	/**
	 * Simple line from a to b, note that the associated uv coordinates are directly associated
	 * with the x and y position of the line start and end position, the normals are perpendicular
	 * to the direction of the line, ie: cross({0,0,1}, normalize(End-Start)))
	 */
	class NAPAPI Line : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		// Properties
		glm::vec3 mStart =	{ -0.5f, 0.0f, 0.0f };	// Start point of the line
		glm::vec3 mEnd =	{ 0.5f, 0.0f, 0.0f };	// End point of the line
		bool mClosed =		false;					// If the line is closed or not
		
		/**
		 * Creates the line
		 * @return if the line was successfully created
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		// Property: the amount of vertices of this line
		int mVertexCount = 2;
	};


	//////////////////////////////////////////////////////////////////////////


	/**
	 * Simple rectangle as a polygon line on the x / y axis
	 * Vertex count starts at the lower left corner and evolves counter-clockwise
	 * The uv coordinates are normalized based on the longest edge
	 * The normals are interpolated from the edge vertices and point outwards from the center
	 */
	class NAPAPI Rectangle : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		glm::vec2 mDimensions;						// Width / Height of the rectangle 

		/**
		* Creates the rectangle
		* @return if the rectangle was successfully created
		*/
		virtual bool init(utility::ErrorState& errorState) override;
	};


	/**
	 * Simple circle as a polygon line
	 * The uv's are normalized 0-1, the normals point outwards from the center
	 */
	class NAPAPI Circle : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		float mRadius;							// Radius of the circle

		/**
		* Creates the rectangle
		* @return if the rectangle was successfully created
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		// property: the number of segments
		int mSegments = 100;
	};

	
	/**
	 * Simple hexagon as a polygon line
	 * The uv's are normalized 0-1, the normals point outwards from the center
	 */
	class NAPAPI Hexagon : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		float mRadius = 1.0f;						// Radius of the hexagon

		/**
		 * Creates the hexagon
		 * @return if the hexagon was successfully created
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};

	
	/**
	 * Simple equal sided triangle
	 * The uv's are normalized 0-1, the normals point outwards from the center
	 */
	class NAPAPI Triangle : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		float mRadius = 1.0f;							// Size of the triangle

		/**
		 * Creates the equal sided triangle
		 * @return if the triangle mesh was created successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;
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