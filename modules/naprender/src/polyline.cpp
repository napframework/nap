// Local Includes
#include "renderservice.h"
#include "polyline.h"

// External Includes
#include <mathutils.h>
#include <glm/gtx/rotate_vector.hpp>
#include <meshutils.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::PolyLineProperties)
	RTTI_PROPERTY("Color",		&nap::PolyLineProperties::mColor,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage",		&nap::PolyLineProperties::mUsage,	nap::rtti::EPropertyMetaData::Default)	
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PolyLine)
	RTTI_PROPERTY("Properties",	&nap::PolyLine::mLineProperties,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Line)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Start",		&nap::Line::mStart,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("End",		&nap::Line::mEnd,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Closed",		&nap::Line::mClosed,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Vertices",	&nap::Line::mVertexCount,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Rectangle)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Dimensions", &nap::Rectangle::mDimensions,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Circle)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Radius",		&nap::Circle::mRadius,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Segments",	&nap::Circle::mSegments,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Hexagon)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Radius",		&nap::Hexagon::mRadius,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TriangleLine)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Radius",		&nap::TriangleLine::mRadius, nap::rtti::EPropertyMetaData::Default)
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
			float const y = cos(2 * M_PI * r * R);
			float const x = sin(2 * M_PI * r * R);

			// Set texture coordinates
			*t++ = {(x + 1.0f) / 2.0f, (y + 1.0) / 2.0f, 0.0f};

			// Set vertex coordinates
			*v++ = { x * radius, y * radius, 0.0f };

			// Set normal coordinates
			*n++ = { x, y, 0.0f };

			// Set color coordinates
			*c++ = color;
		}
	}


	void nap::PolyLine::createVertexAttributes(MeshInstance& instance)
	{
		// Create attributes
		instance.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		instance.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		instance.getOrCreateAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));
		instance.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
	}


	// Initialize base class
	bool nap::PolyLine::init(utility::ErrorState& errorState)
	{
		// Create the mesh	
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<nap::MeshInstance>(mRenderService);
		mMeshInstance->setUsage(mLineProperties.mUsage);
		mMeshInstance->setDrawMode(EDrawMode::LineStrip);

		// Create attributes
		createVertexAttributes(*mMeshInstance);

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
		int p_count = math::resampleLine<glm::vec3>(verts, getPositionAttr().getData(), mVertexCount, mClosed);

		// Set color buffer
		std::vector<glm::vec4> colors(p_count, mLineProperties.mColor);
		getColorAttr().setData(colors);

		// Calculate normal
		glm::vec3 n = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(mEnd - mStart));
		std::vector<glm::vec3> n_verts = { n, n };
		math::resampleLine<glm::vec3>(n_verts, getNormalAttr().getData(), mVertexCount, mClosed);

		// Set normalized uvs
		glm::vec3 uv_end_offset = mEnd - mStart;
		glm::vec3 d_n = normalize(uv_end_offset);
		std::vector<glm::vec3> uv_coords = { { 0.0f,0.0f,0.0f }, d_n };

		// Upsample line
		math::resampleLine<glm::vec3>(uv_coords, getUvAttr().getData(), mVertexCount, mClosed);
		mMeshInstance->setNumVertices(p_count);

		MeshShape& shape = mMeshInstance->createShape();
		utility::generateIndices(shape, p_count, mClosed);

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
		p_verts[0] = { 0.0f - dx, 0.0f + dy, 0.0f };
		p_verts[1] = { 0.0f + dx, 0.0f + dy, 0.0f };
		p_verts[2] = { 0.0f + dx, 0.0f - dy, 0.0f };
		p_verts[3] = { 0.0f - dx, 0.0f - dy, 0.0f };

		// Set positions
		getPositionAttr().setData(p_verts);

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
		uv_verts[0] = { 0.5f - dsx, 0.5f + dsy, 0.0f };
		uv_verts[1] = { 0.5f + dsx, 0.5f + dsy, 0.0f };
		uv_verts[2] = { 0.5f + dsx, 0.5f - dsy, 0.0f };
		uv_verts[3] = { 0.5f - dsx, 0.5f - dsy, 0.0f };

		// Set uv buffer based on normalized uv coordinates
		getUvAttr().setData(uv_verts);

		// Set color buffer
		std::vector<glm::vec4> colors(4, mLineProperties.mColor);
		getColorAttr().setData(colors);

		// Set normal buffer
		std::vector <glm::vec3> n_verts(4);
		getCentroidNormals(p_verts, n_verts, { 0.0f,0.0f,0.0f });
		getNormalAttr().setData(n_verts);

		// Update mesh vertex count
		mMeshInstance->setNumVertices(4);
		MeshShape& shape = mMeshInstance->createShape();
		utility::generateIndices(shape, 4, true);

		// Initialize line
		bool success = mMeshInstance->init(errorState);

		return success;
	}

	//////////////////////////////////////////////////////////////////////////

	bool Circle::init(utility::ErrorState& errorState)
	{
		if (!PolyLine::init(errorState))
			return false;

		// Create the circle
		createCircle(mSegments, mRadius, mLineProperties.mColor, 
			getPositionAttr().getData(),
			getNormalAttr().getData(),
			getColorAttr().getData(),
			getUvAttr().getData());

		// Update
		mMeshInstance->setNumVertices(mSegments);
		MeshShape& shape = mMeshInstance->createShape();
		utility::generateIndices(shape, mSegments, true);		

		// Initialize line
		return mMeshInstance->init(errorState);
	}


	Hexagon::Hexagon(nap::Core& core) : PolyLine(core)
	{ }


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
		getPositionAttr().setData(pos);

		// Resample and set uvs
		getUvAttr().setData(uvs);

		// Resample and set normals
		getNormalAttr().setData(normals);

		// Set color buffer
		getColorAttr().setData(colors);

		// Update
		mMeshInstance->setNumVertices(6);
		MeshShape& shape = mMeshInstance->createShape();
		utility::generateIndices(shape, 6, true);

		return mMeshInstance->init(errorState);
	}


	bool TriangleLine::init(utility::ErrorState& errorState)
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
		getPositionAttr().setData(pos);

		// Resample and set uvs
		getUvAttr().setData(uvs);

		// Resample and set normals
		getNormalAttr().setData(normals);

		// Set color buffer
		getColorAttr().setData(colors);

		// Update
		mMeshInstance->setNumVertices(3);
		mMeshInstance->setDrawMode(EDrawMode::LineStrip);

		MeshShape& shape = mMeshInstance->createShape();
		utility::generateIndices(shape, 3, true);

		return mMeshInstance->init(errorState);
	}


	PolyLine::PolyLine(nap::Core& core) : mRenderService(core.getService<nap::RenderService>())
	{ }


	Line::Line(nap::Core& core) : PolyLine(core)
	{ }


	Rectangle::Rectangle(nap::Core& core) : PolyLine(core)
	{ }


	Circle::Circle(nap::Core& core) : PolyLine(core)
	{ }


	TriangleLine::TriangleLine(nap::Core& core) : PolyLine(core)
	{ }


	Vec3VertexAttribute& PolyLine::getPositionAttr()
	{
		return getMeshInstance().getAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
	}


	const nap::Vec3VertexAttribute& PolyLine::getPositionAttr() const
	{
		return getMeshInstance().getAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
	}


	Vec4VertexAttribute& PolyLine::getColorAttr()
	{
		return getMeshInstance().getAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));
	}


	const nap::Vec4VertexAttribute& PolyLine::getColorAttr() const
	{
		return getMeshInstance().getAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));
	}


	Vec3VertexAttribute& PolyLine::getNormalAttr()
	{
		return getMeshInstance().getAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
	}


	const nap::Vec3VertexAttribute& PolyLine::getNormalAttr() const
	{
		return getMeshInstance().getAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
	}


	Vec3VertexAttribute& PolyLine::getUvAttr()
	{
		return getMeshInstance().getAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
	}


	const nap::Vec3VertexAttribute& PolyLine::getUvAttr() const
	{
		return getMeshInstance().getAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
	}


	void PolyLine::getNormal(const std::map<float, int>& distanceMap, const Vec3VertexAttribute& attr, float location, glm::vec3& outValue) const
	{
		return math::getNormalAlongLine(distanceMap, attr.getData(), location, outValue);
	}


	float PolyLine::getDistances(std::map<float, int>& outDistances) const
	{
		return math::getDistancesAlongLine(getPositionAttr().getData(), outDistances, isClosed());
	}
}