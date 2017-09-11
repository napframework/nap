#pragma once

#include "mesh.h"

namespace nap
{
	/**
	 * Properties commonly associated with a PolyLine
	 */
	struct PolyLineProperties
	{
		int mVertices = 10;								// Total number of vertices that make up the line
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
		virtual const MeshInstance&	getMeshInstance() const override			{ return *mMeshInstance; }

		/**
		 *	@return the position vertex data
		 */
		Vec3VertexAttribute& getPositionAttr();

		/**
		 * @return the position of a vertex along the line @location
		 * @param parametric location (0-1) on the line to get the position for
		 * @param outPosition the interpolated position based on @location
		 */
		void getPosition(float location, glm::vec3& outPosition);
		
		/**
		 *	@return the color vertex data
		 */
		Vec4VertexAttribute& getColorAttr();

		/**
		* @return the color of a vertex along the line @location
		* @param parametric location (0-1) on the line to get the color for
		* @param outColor the interpolated color based on @location
		*/
		void getColor(float location, glm::vec4& outColor);
		
		/**
		 *	@return the normal data
		 */
		Vec3VertexAttribute& getNormalAttr();

		/**
		* @return the normal of a vertex along the line @location
		* @param parametric location (0-1) on the line to get the normal for
		* @param outNormal the interpolated normal based on @location
		*/
		void getNormal(float location, glm::vec3& outNormal);

		/**
		* @return the uv of a vertex along the line @location
		* @param parametric location (0-1) on the line to get the uv for
		* @param outUv the interpolated uv based on @location
		*/
		void getUv(float location, glm::vec3& outUv);
		
		/**
		 *	@return the uv data
		 */
		Vec3VertexAttribute& getUvAttr();

		/**
		 *	@return if the line is closed or not
		 */
		bool isClosed() const;

		/**
		 * Returns an interpolated value of an attribute along the line based on @location
		 * @param attr the polyline attribute to sample
		 * @param parametric normalized location along the spline
		 * @param if the line is closed or not
		 * @param outValue the interpolated value
		 */
		template<typename T> 
		static void getInterpolatedValue(const VertexAttribute<T>& attr, float location, bool closed, T& outValue);
		

		// Properties associated with a line
		PolyLineProperties mLineProperties;

	protected:
		std::unique_ptr<MeshInstance> mMeshInstance;

		// Creates the default vertex line attributes, useful for easy access
		void createVertexAttributes();

		Vec3VertexAttribute* mPositions = nullptr;		// Vertex positions of the line
		Vec4VertexAttribute* mColors = nullptr;			// Colors of the line
		Vec3VertexAttribute* mNormals = nullptr;		// Normals of the line
		Vec3VertexAttribute* mUvs = nullptr;			// Uvs of the line
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
	void nap::PolyLine::getInterpolatedValue(const VertexAttribute<T>& attr, float location, bool closed, T& outValue)
	{
		// Get vertex range, when the line is closed it means that we want to
		// include the first vertex as last
		int vert_count = static_cast<int>(attr.getSize());
		assert(vert_count > 0);
		int range = closed ? vert_count : vert_count - 1;

		// Get interpolation location
		float loc = location * static_cast<float>(range);

		// Get min and max point bounds, wrap the last vertex if the line is closed
		// ie: the last vertex is the first vertex
		int min_vertex = static_cast<int>(math::floor<float>(loc));
		int max_vertex = static_cast<int>(math::ceil<float>(loc));

		min_vertex = min_vertex % vert_count;
		max_vertex = max_vertex % vert_count;

		// Get lerp value that sits in between min / max
		float lerp_v = loc - static_cast<float>(static_cast<int>(loc));

		// Ensure points are in bounds
		assert(min_vertex <= vert_count-1);
		assert(max_vertex <= vert_count-1);

		// Lerp between min and max value
		const T& min_p_value = attr.getData()[min_vertex];
		const T& max_p_value = attr.getData()[max_vertex];

		outValue = math::lerp<T>(min_p_value, max_p_value, lerp_v);
	}

}