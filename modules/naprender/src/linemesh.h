#pragma once

#include "mesh.h"

namespace nap
{
	/**
	 * Properties commonly associated with a line mesh
	 */
	struct LineMeshProperties
	{
		int mVertices = 10;								// Total number of vertices that make up the line
		glm::vec4 mColor = { 1.0f, 1.0f, 1.0f,1.0f };	// Color of the line
	};


	/**
	 * Base class for all line mesh types
	 * These lines are utility classes for constructing simple polyline objects
	 * Every line type is constructed using a number of vertices and has a
	 * color, uv and normal vertex attribute
	 * And / or if the shape is closed / open
	 */
	class NAPAPI PolyLine : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		/**
		 *	Create the base mesh
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		virtual MeshInstance&			getMeshInstance() override				{ return *mMeshInstance; }
		virtual const MeshInstance&		getMeshInstance() const override		{ return *mMeshInstance; }

		// Utilities
		TypedVertexAttribute<glm::vec3>* getPositionData() const				{ return mPostionAttr; }
		TypedVertexAttribute<glm::vec4>* getColorData() const					{ return mColorAttr; }
		TypedVertexAttribute<glm::vec3>* getNormalData() const					{ return mNormalAttr; }
		TypedVertexAttribute<glm::vec3>* getUvData() const						{ return mUvAttr; }

		// Properties associated with a line
		LineMeshProperties mLineProperties;

	protected:
		std::unique_ptr<MeshInstance> mMeshInstance;

		// Creates line, needs to be implemented by derived classes
		virtual void createLine() = 0;

		// Creates the default vertex line attributes, useful for easy access
		void createVertexAttributes();

		// The various line attributes that can be used in the rendering pipe
		TypedVertexAttribute<glm::vec3>* mPostionAttr = nullptr;	// position
		TypedVertexAttribute<glm::vec3>* mUvAttr = nullptr;			// uv
		TypedVertexAttribute<glm::vec4>* mColorAttr = nullptr;		// color
		TypedVertexAttribute<glm::vec3>* mNormalAttr = nullptr;		// normal
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
		

		// Creates the line based on start / end points of the line
		virtual void createLine() override;
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

		// Creates the line based on the rectangle dimensions
		virtual void createLine() override;
	};


	/**
	 *	Simple circle as a polygon line
	 */
	class NAPAPI Circle : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		float mRadius;							// Radius of the circle

		// Creates the circle based on the radius
		virtual void createLine() override;
	};
}