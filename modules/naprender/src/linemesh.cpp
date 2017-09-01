#include "linemesh.h"

RTTI_BEGIN_CLASS(nap::LineMeshProperties)
	RTTI_PROPERTY("Count",		&nap::LineMeshProperties::mVertices,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color",		&nap::LineMeshProperties::mColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Closed",		&nap::LineMeshProperties::mClosed,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PolyLine)
	RTTI_PROPERTY("Properties",	&nap::PolyLine::mLineProperties,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Line)
	RTTI_PROPERTY("Start",		&nap::Line::mStart,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("End",		&nap::Line::mEnd,						nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Rectangle)
	RTTI_PROPERTY("Dimensions", &nap::Rectangle::mDimensions,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Circle)
	RTTI_PROPERTY("Radius",		&nap::Circle::mRadius,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	static int upsampleLine(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& buffer, int points, bool closed)
	{
		assert(points > 0);
		int vertex_count = static_cast<int>(vertices.size());

		// If there not enough or an equal amount or less vertices, don't do anything
		if (points <= vertex_count || vertex_count < 2)
		{
			buffer = vertices;
			return vertices.size();
		}

		// Figure out the amount of edges, closed lines have one extra edge (connecting first to last)
		int edge_count = closed ? vertex_count : vertex_count - 1;

		// Calculate the total amount of pointer for every side
		int pps = points / edge_count;

		// Clear existing buffer data
		buffer.clear();

		// Reserve space for points to add
		buffer.reserve(points);

		for (int i = 0; i < edge_count; i++)
		{
			// Get edge points
			glm::vec3 point_one = vertices[i];
			glm::vec3 point_two = i + 1 >= vertex_count ? vertices[0] : vertices[i + 1];

			// calculate point to point normal
			glm::vec3 normal = glm::normalize(point_two - point_one);

			// step increment
			float inc = glm::distance(point_two, point_one) / float(pps);

			// Add edge vertices
			for (int p = 0; p < pps; p++)
			{
				glm::vec3 new_point = point_one + (normal * float(p) * inc);
				{
					buffer.emplace_back(new_point);
				}
			}
		}
		return buffer.size();
	}


	void nap::PolyLine::createVertexAttributes()
	{
		// Create attributes
		mPostionAttr = &(mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName()));
		mUvAttr	= &(mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetUVName(0)));
		mColorAttr = &(mMeshInstance->GetOrCreateAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0)));
		mNormalAttr = &(mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::getNormalName()));
	}


	// Initialize base class
	bool nap::PolyLine::init(utility::ErrorState& errorState)
	{
		// Create the mesh
		mMeshInstance = std::make_unique<nap::MeshInstance>();

		// Create attributes
		createVertexAttributes();

		// Create line
		createLine();

		// Update instance properties
		mMeshInstance->setDrawMode(mLineProperties.mClosed ? opengl::EDrawMode::LINE_LOOP : opengl::EDrawMode::LINE_STRIP);

		// Initialize line
		return mMeshInstance->init(errorState);
	}


	//////////////////////////////////////////////////////////////////////////


	void Line::createLine()
	{
		// Add initial points
		std::vector<glm::vec3> verts = { mStart, mEnd };

		// Populate position buffer with data
		int p_count = upsampleLine(verts, mPostionAttr->getValues(), mLineProperties.mVertices, mLineProperties.mClosed);

		// Set color buffer
		mColorAttr->setValues(std::vector<glm::vec4>(p_count, mLineProperties.mColor));

		// Set normal buffer
		mNormalAttr->setValues(std::vector<glm::vec3>(p_count, { 0.0f, 0.0f, 0.0f }));

		// Set uvs, todo: normalize them
		upsampleLine(verts, mUvAttr->getValues(), mLineProperties.mVertices, mLineProperties.mClosed);
		mMeshInstance->setNumVertices(p_count);
	}


	//////////////////////////////////////////////////////////////////////////


	void Rectangle::createLine()
	{
		float dx = mDimensions.x / 2.0f;
		float dy = mDimensions.y / 2.0f;

		std::vector<glm::vec3> p_verts(4);
		p_verts[0] = { 0.0f - dx, 0.0f - dy, 0.0f };
		p_verts[1] = { 0.0f + dx, 0.0f - dy, 0.0f };
		p_verts[2] = { 0.0f + dx, 0.0f + dy, 0.0f };
		p_verts[3] = { 0.0f - dx, 0.0f + dy, 0.0f };

		// Set positions
		int pos_count = upsampleLine(p_verts, mPostionAttr->getValues(), mLineProperties.mVertices, mLineProperties.mClosed);
		
		// Set uv buffer (TODO: Make the uv's relative to dimensions and normalized)
		upsampleLine(p_verts, mUvAttr->getValues(), mLineProperties.mVertices, mLineProperties.mClosed);

		// Set color buffer
		mColorAttr->setValues(std::vector<glm::vec4>(pos_count, mLineProperties.mColor));

		// Set normal buffer
		mNormalAttr->setValues(std::vector<glm::vec3>(pos_count, { 0.0f, 0.0f, 0.0f }));
		mMeshInstance->setNumVertices(pos_count);
	}


	//////////////////////////////////////////////////////////////////////////


	void Circle::createLine()
	{
		mPostionAttr->clear();
		mPostionAttr->resize(mLineProperties.mVertices);
		std::vector<glm::vec3>::iterator v = mPostionAttr->getValues().begin();

		mNormalAttr->clear();
		mNormalAttr->resize(mLineProperties.mVertices);
		std::vector<glm::vec3> ::iterator n = mNormalAttr->getValues().begin();

		mColorAttr->clear();
		mColorAttr->resize(mLineProperties.mVertices);
		std::vector<glm::vec4>::iterator c = mColorAttr->getValues().begin();

		mUvAttr->clear();
		mUvAttr->resize(mLineProperties.mVertices);
		std::vector<glm::vec3>::iterator t = mUvAttr->getValues().begin();

		float const R = 1.0f / static_cast<float>(mLineProperties.mVertices - 1);

		for (int r = 0; r < mLineProperties.mVertices; r++)
		{
			float const y = sin(2 * M_PI * r * R);
			float const x = cos(2 * M_PI * r * R);

			// Set texture coordinates
			*t++ = { 1.0f - (r*R), r*R, 0.5f };

			// Set vertex coordinates
			*v++ = { x * mRadius, y * mRadius, 0.0f };

			// Set normal coordinates
			*n++ = { x, y, 0.0f };

			// Set color coordinates
			*c++ = mLineProperties.mColor;
		}

		mMeshInstance->setNumVertices(mLineProperties.mVertices);
	}
}
