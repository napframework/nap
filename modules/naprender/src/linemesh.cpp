#include "linemesh.h"
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::LineMeshProperties)
	RTTI_PROPERTY("Count",		&nap::LineMeshProperties::mVertices,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color",		&nap::LineMeshProperties::mColor,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PolyLine)
	RTTI_PROPERTY("Properties",	&nap::PolyLine::mLineProperties,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Line)
	RTTI_PROPERTY("Start",		&nap::Line::mStart,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("End",		&nap::Line::mEnd,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Closed",		&nap::Line::mClosed,					nap::rtti::EPropertyMetaData::Default)
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
	static void getCentroidNormals(const std::vector<glm::vec3>& positions, std::vector<glm::vec3>& outNormals, const glm::vec3& center)
	{
		outNormals.resize(positions.size());
		int count = 0;
		for (const auto& p : positions)
		{
			outNormals[count] = glm::normalize(p - center);
			count++;
		}
	}

	static int resampleLine(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& buffer, int segments, bool closed, bool isNormal = false)
	{
		assert(segments > 0);
		int vertex_count = static_cast<int>(vertices.size());

		// If there not enough or an equal amount or less vertices, don't do anything
		if (segments <= vertex_count || vertex_count < 2)
		{
			buffer = vertices;
			return vertices.size();
		}

		// Figure out the amount of edges, closed lines have one extra edge (connecting first to last)
		int edge_count = closed ? vertex_count : vertex_count - 1;

		// Calculate the total amount of pointer for every side
		int pps = segments / edge_count;

		// Clear existing buffer data
		buffer.clear();

		// Reserve space for points to add
		buffer.reserve(segments);

		for (int i = 0; i < edge_count; i++)
		{
			// Get edge points
			glm::vec3& point_one = vertices[i];
			glm::vec3& point_two = i + 1 >= vertex_count ? vertices[0] : vertices[i + 1];

			// Add edge vertices
			for (int p = 0; p < pps; p++)
			{
				float inc = static_cast<float>(p) / static_cast<float>(pps);

				float x = nap::math::lerp<float>(point_one.x, point_two.x, inc);
				float y = nap::math::lerp<float>(point_one.y, point_two.y, inc);
				float z = nap::math::lerp<float>(point_one.z, point_two.z, inc);
				buffer.emplace_back(isNormal ? glm::normalize(glm::vec3(x, y, z)) : glm::vec3(x, y, z));
			}
		}

		// If the line is open add an additional point
		if (!closed)
			buffer.emplace_back(vertices.back());

		// Return total number of new points
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

		// Initialize line
		return mMeshInstance->init(errorState);
	}


	//////////////////////////////////////////////////////////////////////////


	void Line::createLine()
	{
		// Add initial points
		std::vector<glm::vec3> verts = { mStart, mEnd };

		// Populate position buffer with data
		int p_count = resampleLine(verts, mPostionAttr->getValues(), mLineProperties.mVertices, mClosed);

		// Set color buffer
		mColorAttr->setValues(std::vector<glm::vec4>(p_count, mLineProperties.mColor));

		// Calculate normal
		glm::vec3 n = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(mEnd - mStart));
		std::vector<glm::vec3> n_verts = { n, n };
		resampleLine(n_verts, mNormalAttr->getValues(), mLineProperties.mVertices, mClosed, true);

		// Set uvs, todo: normalize them
		glm::vec3 uv_end_offset = mEnd - mStart;
		glm::vec3 d_n = normalize(uv_end_offset);
		std::vector<glm::vec3> uv_coords = { {0.0f,0.0f,0.0f}, d_n };

		resampleLine(uv_coords, mUvAttr->getValues(), mLineProperties.mVertices, mClosed);
		mMeshInstance->setNumVertices(p_count);

		// Set draw mode
		mMeshInstance->setDrawMode(mClosed ? opengl::EDrawMode::LINE_LOOP : opengl::EDrawMode::LINE_STRIP);
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
		int pos_count = resampleLine(p_verts, mPostionAttr->getValues(), mLineProperties.mVertices, true);
		
		// Calculate normalized UV coordinates
		float sx, sy;
		if (dx > dy)
		{
			sx = 1.0f;
			sy = dy / dx;
		}
		else
		{
			sx = dx / dy;
			sy = 1.0f;
		}
		float dsx = sx / 2.0f;
		float dsy = sy / 2.0f;


		std::vector<glm::vec3> uv_verts(4);
		uv_verts[0] = { 0.5f - dsx, 0.5f - dsy, 0.0f };
		uv_verts[1] = { 0.5f + dsx, 0.5f - dsy, 0.0f };
		uv_verts[2] = { 0.5f + dsx, 0.5f + dsy, 0.0f };
		uv_verts[3] = { 0.5f - dsx, 0.5f + dsy, 0.0f };

		// Set uv buffer based on normalized uv coordinates
		resampleLine(uv_verts, mUvAttr->getValues(), mLineProperties.mVertices, true);

		// Set color buffer
		mColorAttr->setValues(std::vector<glm::vec4>(pos_count, mLineProperties.mColor));

		// Set normal buffer
		std::vector <glm::vec3> n_verts(4);
		getCentroidNormals(p_verts, n_verts, { 0.0f,0.0f,0.0f });
		resampleLine(n_verts, mNormalAttr->getValues(), mLineProperties.mVertices, true, true);

		// Update mesh vertex count
		mMeshInstance->setNumVertices(pos_count);

		// Set draw mode
		mMeshInstance->setDrawMode(opengl::EDrawMode::LINE_LOOP);
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
		mMeshInstance->setDrawMode(opengl::EDrawMode::LINE_LOOP);
	}
}
