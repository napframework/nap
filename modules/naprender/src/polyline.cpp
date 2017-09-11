#include "polyline.h"
#include <mathutils.h>
#include <glm/gtx/rotate_vector.hpp>

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

RTTI_BEGIN_CLASS(nap::Hexagon)
	RTTI_PROPERTY("Radius",		&nap::Hexagon::mRadius,					nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Triangle)
	RTTI_PROPERTY("Radius",		&nap::Triangle::mRadius,				nap::rtti::EPropertyMetaData::Default)
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
	 * @param vertices the original mesh verticese
	 * @param buffer the buffer that will hold the resampled vertices
	 * @param segments the amount of segments the poly line should have, a segment is the line between two vertices
	 * @param closed if the line is closed or not, if a line is not closed it will contain one extra point to maintain the last vertex
	 * @param isNormal if the attribute is a normal, in that case the interpolation will return a normalized value (copied from Houdini)
	 * @return the total number of generated vertics
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


	/**
	 * Creates a circle that consists out of @vertexCount vertices
	 * The circle is considered close where the last vertex is the one before the start
	 * @param vertexCount the number of circle vertices
	 * @param radius the radius of the circle
	 * @param color the color of the circle
	 * @param out*& the buffers to fill based on the amount of vertices
	 */
	static void createCircle(int vertexCount, float radius, const glm::vec4& color, 
		std::vector<glm::vec3>& outPos,
		std::vector<glm::vec3>& outNormal,
		std::vector<glm::vec4>& outColor,
		std::vector<glm::vec3>& outUv)
	{
		//nap::VertexAttribute<glm::vec3>& pos_data = outMesh.getPositionAttr();
		outPos.clear();
		outPos.resize(vertexCount);
		std::vector<glm::vec3>::iterator v = outPos.begin();

		//nap::VertexAttribute<glm::vec3>& normal_data = outMesh.getNormalAttr();
		outNormal.clear();
		outNormal.resize(vertexCount);
		std::vector<glm::vec3> ::iterator n = outNormal.begin();

		//nap::VertexAttribute<glm::vec4>& color_data = outMesh.getColorAttr();
		outColor.clear();
		outColor.resize(vertexCount);
		std::vector<glm::vec4>::iterator c = outColor.begin();

		//nap::VertexAttribute<glm::vec3>& uv_data = outMesh.getUvAttr();
		outUv.clear();
		outUv.resize(vertexCount);
		std::vector<glm::vec3>::iterator t = outUv.begin();

		float const R = 1.0f / static_cast<float>(vertexCount);

		// Sweep and fill
		for (int r = 0; r < vertexCount; r++)
		{
			float const y = sin(2 * M_PI * r * R);
			float const x = cos(2 * M_PI * r * R);

			// Set texture coordinates
			*t++ = { 1.0f - (r*R), r*R, 0.5f };

			// Set vertex coordinates
			*v++ = { x * radius, y * radius, 0.0f };

			// Set normal coordinates
			*n++ = { x, y, 0.0f };

			// Set color coordinates
			*c++ = color;
		}
	}


	void nap::PolyLine::createVertexAttributes()
	{
		// Create attributes
		mPositions = &(mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName()));
		mUvs = &(mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetUVName(0)));
		mColors = &(mMeshInstance->GetOrCreateAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0)));
		mNormals = &(mMeshInstance->GetOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::getNormalName()));
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
		int p_count = resampleLine(verts, getPositionAttr().getData(), mLineProperties.mVertices, mClosed);

		// Set color buffer
		std::vector<glm::vec4> colors(p_count, mLineProperties.mColor);
		getColorAttr().setData(colors);

		// Calculate normal
		glm::vec3 n = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(mEnd - mStart));
		std::vector<glm::vec3> n_verts = { n, n };
		resampleLine(n_verts, getNormalAttr().getData(), mLineProperties.mVertices, mClosed, true);

		// Set normalized uvs
		glm::vec3 uv_end_offset = mEnd - mStart;
		glm::vec3 d_n = normalize(uv_end_offset);
		std::vector<glm::vec3> uv_coords = { { 0.0f,0.0f,0.0f }, d_n };

		// Upsample line
		resampleLine(uv_coords, getUvAttr().getData(), mLineProperties.mVertices, mClosed);
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
		int pos_count = resampleLine(p_verts, getPositionAttr().getData(), mLineProperties.mVertices, true);

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
		resampleLine(uv_verts, getUvAttr().getData(), mLineProperties.mVertices, true);

		// Set color buffer
		std::vector<glm::vec4> colors(pos_count, mLineProperties.mColor);
		getColorAttr().setData(colors);

		// Set normal buffer
		std::vector <glm::vec3> n_verts(4);
		getCentroidNormals(p_verts, n_verts, { 0.0f,0.0f,0.0f });
		resampleLine(n_verts, getNormalAttr().getData(), mLineProperties.mVertices, true, true);

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

		// Create the circle
		createCircle(mLineProperties.mVertices, mRadius, mLineProperties.mColor, 
			getPositionAttr().getData(),
			getNormalAttr().getData(),
			getColorAttr().getData(),
			getUvAttr().getData());

		// Update
		mMeshInstance->setNumVertices(mLineProperties.mVertices);
		mMeshInstance->setDrawMode(opengl::EDrawMode::LINE_LOOP);

		// Initialize line
		return mMeshInstance->init(errorState);
	}


	bool Hexagon::init(utility::ErrorState& errorState)
	{
		if (!PolyLine::init(errorState))
			return false;

		std::vector<glm::vec3> pos;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec3> uvs;
		
		// Create a circle that consists out of 6 vertices
		createCircle(6, mRadius, mLineProperties.mColor, pos, normals, colors, uvs);

		// Resample the circle to have equal amount of points
		int vert_count = resampleLine(pos, getPositionAttr().getData(), mLineProperties.mVertices, true, false);

		// Resample and set uvs
		resampleLine(uvs, getUvAttr().getData(), mLineProperties.mVertices, true, false);

		// Resample and set normals
		resampleLine(normals, getNormalAttr().getData(), mLineProperties.mVertices, true, true);

		// Set color buffer
		std::vector<glm::vec4> vert_colors(vert_count, mLineProperties.mColor);
		getColorAttr().setData(vert_colors);

		// Update
		mMeshInstance->setNumVertices(vert_count);
		mMeshInstance->setDrawMode(opengl::EDrawMode::LINE_LOOP);

		return mMeshInstance->init(errorState);
	}


	bool Triangle::init(utility::ErrorState& errorState)
	{
		if (!PolyLine::init(errorState))
			return false;

		std::vector<glm::vec3> pos;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec4> colors;
		std::vector<glm::vec3> uvs;

		// Create a circle that consists out of 6 vertices
		createCircle(3, mRadius, mLineProperties.mColor, pos, normals, colors, uvs);

		// Resample the circle to have equal amount of points
		int vert_count = resampleLine(pos, getPositionAttr().getData(), mLineProperties.mVertices, true, false);

		// Resample and set uvs
		resampleLine(uvs, getUvAttr().getData(), mLineProperties.mVertices, true, false);

		// Resample and set normals
		resampleLine(normals, getNormalAttr().getData(), mLineProperties.mVertices, true, true);

		// Set color buffer
		std::vector<glm::vec4> vert_colors(vert_count, mLineProperties.mColor);
		getColorAttr().setData(vert_colors);

		// Update
		mMeshInstance->setNumVertices(vert_count);
		mMeshInstance->setDrawMode(opengl::EDrawMode::LINE_LOOP);

		return mMeshInstance->init(errorState);

	}


	Vec3VertexAttribute& PolyLine::getPositionAttr()
	{
		assert(mPositions != nullptr);
		return *mPositions;
	}


	void PolyLine::getPosition(float location, glm::vec3& outPosition) const
	{
		PolyLine::getValueAlongLine<glm::vec3>(*mPositions, location, isClosed(), outPosition);
	}


	Vec4VertexAttribute& PolyLine::getColorAttr()
	{
		assert(mColors != nullptr);
		return *mColors;
	}


	void PolyLine::getColor(float location, glm::vec4& outColor) const
	{
		PolyLine::getValueAlongLine<glm::vec4>(*mColors, location, isClosed(), outColor);
	}


	Vec3VertexAttribute& PolyLine::getNormalAttr()
	{
		assert(mNormals != nullptr);
		return *mNormals;
	}


	void PolyLine::getNormal(float location, glm::vec3& outNormal) const
	{
		PolyLine::getValueAlongLine<glm::vec3>(*mNormals, location, isClosed(), outNormal);
	}


	Vec3VertexAttribute& PolyLine::getUvAttr()
	{
		assert(mUvs != nullptr);
		return *mUvs;
	}


	void PolyLine::getUv(float location, glm::vec3& outUv) const
	{
		PolyLine::getValueAlongLine<glm::vec3>(*mUvs, location, isClosed(), outUv);
	}


	bool PolyLine::isClosed() const
	{
		opengl::EDrawMode mode = mMeshInstance->getDrawMode();
		assert(mode == opengl::EDrawMode::LINE_LOOP || mode == opengl::EDrawMode::LINE_STRIP);
		return mode == opengl::EDrawMode::LINE_LOOP;
	}

}