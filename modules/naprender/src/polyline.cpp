#include "polyline.h"
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::PolyLineProperties)
	RTTI_PROPERTY("Count",		&nap::PolyLineProperties::mVertices,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Color",		&nap::PolyLineProperties::mColor,		nap::rtti::EPropertyMetaData::Default)
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
	/**
	 * Utility function to create a set of normals based on a set vertex positions and their distance to @center
	 * @param positions the vertex positions
	 * @param outNormals the buffer that will be filled with the new normals
	 * @param center the position from where to calculate direction
	 */
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

	/**
	 * Utility function to upsample a poly line to @segments
	 * This function distributes the vertices equally among every segment, something that is not desirable with more uneven, complex lines
	 * A weighted distribution method is preferred but for now this will do. 
	 * @param vertices the original mesh vertices
	 * @param buffer the buffer that will hold the resampled vertices
	 * @param segments the amount of segments the poly line should have, a segment is the line between two vertices
	 * @param closed if the line is not, if a line is not closed it will contain one extra point to maintain the last vertex
	 * @param isNormal if the attribute is a normal, in that case the interpolation will return a normalized value (copied from Houdini)
	 * @return the total number of generated vertices
	 */
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
		mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName());
		mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetUVName(0));
		mMeshInstance->GetOrCreateAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0));
		mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::getNormalName());
	}


	// Initialize base class
	bool nap::PolyLine::init(utility::ErrorState& errorState)
	{
		// Create the mesh	
		mMeshInstance = std::make_unique<nap::MeshInstance>();

		// Create attributes
		createVertexAttributes();

		return true;
	}


	//////////////////////////////////////////////////////////////////////////

	bool Line::init(utility::ErrorState& errorState)
	{
		if (!PolyLine::init(errorState))
			return false;

		// Add initial points
		std::vector<glm::vec3> verts = { mStart, mEnd };

		// Populate position buffer with data
		int p_count = resampleLine(verts, getPositionData().getValues(), mLineProperties.mVertices, mClosed);

		// Set color buffer
		std::vector<glm::vec4> colors(p_count, mLineProperties.mColor);
		getColorData().setValues(colors);

		// Calculate normal
		glm::vec3 n = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(mEnd - mStart));
		std::vector<glm::vec3> n_verts = { n, n };
		resampleLine(n_verts, getNormalData().getValues(), mLineProperties.mVertices, mClosed, true);

		// Set normalized uvs
		glm::vec3 uv_end_offset = mEnd - mStart;
		glm::vec3 d_n = normalize(uv_end_offset);
		std::vector<glm::vec3> uv_coords = { { 0.0f,0.0f,0.0f }, d_n };

		// Upsample line
		resampleLine(uv_coords, getUvData().getValues(), mLineProperties.mVertices, mClosed);
		mMeshInstance->setNumVertices(p_count);

		// Set draw mode
		mMeshInstance->setDrawMode(mClosed ? opengl::EDrawMode::LINE_LOOP : opengl::EDrawMode::LINE_STRIP);

		// Initialize line
		return mMeshInstance->init(errorState);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	bool Rectangle::init(utility::ErrorState& errorState)
	{
		if (!PolyLine::init(errorState))
			return false;

		float dx = mDimensions.x / 2.0f;
		float dy = mDimensions.y / 2.0f;

		std::vector<glm::vec3> p_verts(4);
		p_verts[0] = { 0.0f - dx, 0.0f - dy, 0.0f };
		p_verts[1] = { 0.0f + dx, 0.0f - dy, 0.0f };
		p_verts[2] = { 0.0f + dx, 0.0f + dy, 0.0f };
		p_verts[3] = { 0.0f - dx, 0.0f + dy, 0.0f };

		// Set positions
		int pos_count = resampleLine(p_verts, getPositionData().getValues(), mLineProperties.mVertices, true);

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
		resampleLine(uv_verts, getUvData().getValues(), mLineProperties.mVertices, true);

		// Set color buffer
		std::vector<glm::vec4> colors(pos_count, mLineProperties.mColor);
		getColorData().setValues(colors);

		// Set normal buffer
		std::vector <glm::vec3> n_verts(4);
		getCentroidNormals(p_verts, n_verts, { 0.0f,0.0f,0.0f });
		resampleLine(n_verts, getNormalData().getValues(), mLineProperties.mVertices, true, true);

		// Update mesh vertex count
		mMeshInstance->setNumVertices(pos_count);

		// Set draw mode
		mMeshInstance->setDrawMode(opengl::EDrawMode::LINE_LOOP);

		// Initialize line
		return mMeshInstance->init(errorState);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	bool Circle::init(utility::ErrorState& errorState)
	{
		if (!PolyLine::init(errorState))
			return false;

		nap::TypedVertexAttribute<glm::vec3>& pos_data = getPositionData();
		pos_data.clear();
		pos_data.resize(mLineProperties.mVertices);
		std::vector<glm::vec3>::iterator v = pos_data.getValues().begin();

		nap::TypedVertexAttribute<glm::vec3>& normal_data = getNormalData();
		normal_data.clear();
		normal_data.resize(mLineProperties.mVertices);
		std::vector<glm::vec3> ::iterator n = normal_data.getValues().begin();

		nap::TypedVertexAttribute<glm::vec4>& color_data = getColorData();
		color_data.clear();
		color_data.resize(mLineProperties.mVertices);
		std::vector<glm::vec4>::iterator c = color_data.getValues().begin();

		nap::TypedVertexAttribute<glm::vec3>& uv_data = getUvData();
		uv_data.clear();
		uv_data.resize(mLineProperties.mVertices);
		std::vector<glm::vec3>::iterator t = uv_data.getValues().begin();

		float const R = 1.0f / static_cast<float>(mLineProperties.mVertices - 1);

		// Sweep and fill
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

		// Initialize line
		return mMeshInstance->init(errorState);

		return true;
	}


	nap::TypedVertexAttribute<glm::vec3>& PolyLine::getPositionData()
	{
		return mMeshInstance->GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName());
	}


	nap::TypedVertexAttribute<glm::vec4>& PolyLine::getColorData()
	{
		return mMeshInstance->GetAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0));
	}


	nap::TypedVertexAttribute<glm::vec3>& PolyLine::getNormalData()
	{
		return mMeshInstance->GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::getNormalName());
	}


	nap::TypedVertexAttribute<glm::vec3>& PolyLine::getUvData()
	{
		return mMeshInstance->GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetUVName(0));
	}

}