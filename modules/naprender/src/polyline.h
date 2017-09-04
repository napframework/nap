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
	 * Base class for all line mesh types
	 * These lines types are utility classes for constructing simple Poly Line objects
	 * Every line type is constructed using a number of vertices and has a color, uv and normal vertex attribute
	 */
	class NAPAPI PolyLine : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		/**
		 * Create the mesh instance and the necessary vertex attributes (P, Cd, N, Uv)
		 * When implementing a derived PolyLine, make sure to call the base init first
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	@return the mesh associated with this poly line
		 */
		virtual MeshInstance&			getMeshInstance() override				{ return *mMeshInstance; }
		
		/**
		 *	@return const reference to the mesh associated with this poly line
		 */
		virtual const MeshInstance&		getMeshInstance() const override		{ return *mMeshInstance; }

		/**
		 *	@return the position vertex data
		 */
		TypedVertexAttribute<glm::vec3>& getPositionData();
		
		/**
		 *	@return the color vertex data
		 */
		TypedVertexAttribute<glm::vec4>& getColorData();
		
		/**
		 *	@return the normal data
		 */
		TypedVertexAttribute<glm::vec3>& getNormalData();
		
		/**
		 *	@return the uv data
		 */
		TypedVertexAttribute<glm::vec3>& getUvData();

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
	 * with the x and y position of the line start end position
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
	 * Vert count starts at the lower left corner and evolve counter-clockwise
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
	 *	Simple circle as a polygon line
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