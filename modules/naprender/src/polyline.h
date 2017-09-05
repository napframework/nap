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
		 *	@return the color vertex data
		 */
		Vec4VertexAttribute& getColorAttr();
		
		/**
		 *	@return the normal data
		 */
		Vec3VertexAttribute& getNormalAttr();
		
		/**
		 *	@return the uv data
		 */
		Vec3VertexAttribute& getUvAttr();

		// Properties associated with a line
		PolyLineProperties mLineProperties;

	protected:
		std::unique_ptr<MeshInstance> mMeshInstance;

		// Creates the default vertex line attributes, useful for easy access
		void createVertexAttributes();
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
}