// Local includes
#include "linefromfile.h"

// External includes
#include <entity.h>
#include <nanosvg.h>
#include <utility/stringutils.h>
#include <nap/numeric.h>
#include <rect.h>
#include <limits>
#include <meshutils.h>
#include <mathutils.h>
#include <nap/logger.h>
#include <nap/core.h>

RTTI_BEGIN_ENUM(nap::ESVGUnits)
	RTTI_ENUM_VALUE(nap::ESVGUnits::PX,		"px"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::PT,		"pt"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::PC,		"pc"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::MM,		"mm"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::CM,		"cm"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::DPI,	"dpi")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("File",				&nap::LineFromFile::mFile,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Units",				&nap::LineFromFile::mUnits,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DPI",				&nap::LineFromFile::mDPI,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Tolerance",			&nap::LineFromFile::mTolerance, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Normalize",			&nap::LineFromFile::mNormalize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Scale",				&nap::LineFromFile::mScale,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipHorizontal",		&nap::LineFromFile::mFlipX,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipVertical",		&nap::LineFromFile::mFlipY,		nap::rtti::EPropertyMetaData::Default)	
	RTTI_PROPERTY("LineIndex",			&nap::LineFromFile::mLineIndex,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

/**
 *	Calculates the distance of a point to a specific segment on the line
 */
static float distPtSeg(float x, float y, float px, float py, float qx, float qy)
{
	float pqx, pqy, dx, dy, d, t;
	pqx = qx - px;
	pqy = qy - py;
	dx = x - px;
	dy = y - py;
	d = pqx*pqx + pqy*pqy;
	t = pqx*dx + pqy*dy;
	if (d > 0) t /= d;
	if (t < 0) t = 0;
	else if (t > 1) t = 1;
	dx = px + t*pqx - x;
	dy = py + t*pqy - y;
	return dx*dx + dy*dy;
}


/**
 *	Cubic spline bezier implementation
 */
static void cubicBez(float x1, float y1, float x2, float y2,
	float x3, float y3, float x4, float y4,
	float tol, int level, std::vector<glm::vec3>& vertices)
{
	float x12, y12, x23, y23, x34, y34, x123, y123, x234, y234, x1234, y1234;
	float d;

	if (level > 12) return;

	x12		= (x1 + x2)		* 0.5f;
	y12		= (y1 + y2)		* 0.5f;
	x23		= (x2 + x3)		* 0.5f;
	y23		= (y2 + y3)		* 0.5f;
	x34		= (x3 + x4)		* 0.5f;
	y34		= (y3 + y4)		* 0.5f;
	x123	= (x12 + x23)	* 0.5f;
	y123	= (y12 + y23)	* 0.5f;
	x234	= (x23 + x34)	* 0.5f;
	y234	= (y23 + y34)	* 0.5f;
	x1234	= (x123 + x234) * 0.5f;
	y1234	= (y123 + y234) * 0.5f;

	d = distPtSeg(x1234, y1234, x1, y1, x4, y4);
	if (d > tol*tol) 
	{
		cubicBez(x1, y1, x12, y12, x123, y123, x1234, y1234, tol, level + 1, vertices);
		cubicBez(x1234, y1234, x234, y234, x34, y34, x4, y4, tol, level + 1, vertices);
	}
	else 
	{
		vertices.emplace_back(glm::vec3(x4, y4, 0.0f));
	}
}


static void extractPathVertices(float* pts, int npts, char closed, float tol, std::vector<glm::vec3>& vertices)
{
	int i;
	vertices.emplace_back(glm::vec3(pts[0], pts[1], 0.0f));
	for (i = 0; i < npts - 1; i += 3) 
	{
		float* p = &pts[i * 2];
		cubicBez(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], tol, 0, vertices);
	}
}


namespace nap
{
	LineFromFile::LineFromFile(nap::Core& core) : PolyLine(core)
	{}


	bool LineFromFile::init(utility::ErrorState& errorState)
	{
		if (!PolyLine::init(errorState))
			return false;

		// Append . before trying to load relative paths
		std::string img_path = mFile;
		if (utility::startsWith(img_path, "/"))
			img_path = utility::stringFormat(".%s", img_path.c_str());

		// Convert the enum to a string for loading
		rtti::Variant var = mUnits;
		bool conversion_succeeded;
		std::string units = var.to_string(&conversion_succeeded);
		assert(conversion_succeeded);

		NSVGimage* new_image = nsvgParseFromFile(img_path.c_str(), units.c_str(), mDPI);
		if (!errorState.check(new_image != nullptr, "unable to load image: %s", img_path.c_str()))
			return false;

		// Make sure the image contains a shape
		if (!errorState.check(new_image->shapes != nullptr, "image has no shapes: %s", img_path.c_str()))
			return false;

		// Make sure that shape contains a path
		if (!errorState.check(new_image->shapes->paths != nullptr, "image has no paths: %s", img_path.c_str()))
			return false;

		// Container that holds all extracted paths and if they're closed
		SVGPaths extracted_paths;
		SVGState states;

		// Rectangle that is used to compute final image bounds
		math::Rect svg_rect(glm::vec2(math::max<float>(), math::max<float>()), glm::vec2(math::min<float>(), math::min<float>()));

		// Extract all paths
		NSVGshape* current_shape = new_image->shapes;
		while (current_shape != nullptr) 
		{
			NSVGpath* current_path = current_shape->paths;
			while (current_path != nullptr)
			{
				// Extract vertices from current path based on threshold value
				std::unique_ptr<std::vector<glm::vec3>> path_vertices = std::make_unique<std::vector<glm::vec3>>();
				extractPathVertices(current_path->pts, current_path->npts, current_path->closed, mTolerance, *path_vertices);

				// Expand bounds of rectangle
				if (current_path->bounds[0] < svg_rect.mMinPosition.x)
					svg_rect.mMinPosition.x = current_path->bounds[0];

				if (current_path->bounds[1] < svg_rect.mMinPosition.y)
					svg_rect.mMinPosition.y = current_path->bounds[1];

				if (current_path->bounds[2] > svg_rect.mMaxPosition.x)
					svg_rect.mMaxPosition.x = current_path->bounds[2];

				if (current_path->bounds[3] > svg_rect.mMaxPosition.y)
					svg_rect.mMaxPosition.y = current_path->bounds[3];

				// Add path to container
				extracted_paths.emplace_back(std::move(path_vertices));
				bool is_closed = current_path->closed > 0;
				states.emplace_back(is_closed);

				// Cycle to next one
				current_path = current_path->next;
			}
			// Cycle to next shape
			current_shape = current_shape->next;
		}

		// Extract all the lines
		if (!extractLinesFromPaths(extracted_paths, states, svg_rect, errorState))
			return false;

		// Delete the image
		nsvgDelete(new_image);
		return true;
	}


	bool LineFromFile::isClosed(int shapeIndex) const
	{
		assert(shapeIndex < mClosedStates.size());
		return mClosedStates[shapeIndex];
	}


	bool LineFromFile::isClosed() const
	{
		assert(!mClosedStates.empty());
		return mClosedStates.front();
	}


	bool LineFromFile::extractLinesFromPaths(const SVGPaths& paths, const SVGState& states, const math::Rect& rect, utility::ErrorState& errorState)
	{
		// Calculate rectangle ratio
		glm::vec2 ratio(1.0f, 1.0f);
		if (rect.getWidth() < rect.getHeight())
			ratio.x = rect.getWidth() / rect.getHeight();
		else
			ratio.y = rect.getHeight() / rect.getWidth();

		// Calculate uv offset based on rectangle ratio in 0-1 space
		glm::vec2 uvoff(0.0f, 0.0f);
		uvoff.x = (1.0f - ratio.x) / 2.0f;
		uvoff.y = (1.0f - ratio.y) / 2.0f;

		// Calculate scale taking in to account the ratio
		glm::vec2 scale(mScale, mScale);
		if (mNormalize)
		{
			scale = ratio * scale;
		}

		// min - max values for the various buffers
		float uv_x_min = mFlipX ? 1.0f - uvoff.x : 0.0f + uvoff.x;
		float uv_x_max = mFlipX ? 0.0f + uvoff.x : 1.0f - uvoff.x;
		float uv_y_min = mFlipY ? 1.0f - uvoff.y : 0.0f + uvoff.y;
		float uv_y_max = mFlipY ? 0.0f + uvoff.y : 1.0f - uvoff.y;

		// min -max values for the position, first we check if we need to flip, after that if we need to normalize
		float pos_x_min = mFlipX ? mNormalize ? 0.5f  : rect.mMaxPosition.x : mNormalize ? -0.5f : rect.mMinPosition.x;
		float pos_x_max = mFlipX ? mNormalize ? -0.5f : rect.mMinPosition.x : mNormalize ? 0.5f	 : rect.mMaxPosition.x;
		float pos_y_min = mFlipY ? mNormalize ? 0.5f  : rect.mMaxPosition.y : mNormalize ? -0.5f : rect.mMinPosition.y;
		float pos_y_max = mFlipY ? mNormalize ? -0.5f : rect.mMinPosition.y : mNormalize ? 0.5f	 : rect.mMaxPosition.y;

		// Create a set of mesh instances based on those paths
		mClosedStates.clear();
		for (int path_index = 0; path_index != paths.size(); ++path_index)
		{
			auto& path = paths[path_index];

			// Also create uv's and normals based on path vertices
			std::vector<glm::vec3> uvs;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec3> positions;

			// Resize
			uvs.reserve(path->size());
			positions.reserve(path->size());

			// Calculate uv's first as they use the rect to figure out the normalized 0-1 coordinates
			// After that compute the vertex position based on the scale 
			glm::vec3* previous_point = nullptr;
			for (auto& vertex : *path)
			{
				// Discard points that are exactly the same as the previous point
				if (previous_point != nullptr && glm::distance(*previous_point, vertex) <= nap::math::epsilon<float>())
					continue;

				// calculate vertex uv
				float uv_x = nap::math::fit<float>(vertex.x, rect.mMinPosition.x, rect.mMaxPosition.x, uv_x_min, uv_x_max);
				float uv_y = nap::math::fit<float>(vertex.y, rect.mMinPosition.y, rect.mMaxPosition.y, uv_y_min, uv_y_max);
				uvs.emplace_back(glm::vec3(uv_x, uv_y, 0.0f));

				// calculate vertex position
				float pos_x = nap::math::fit<float>(vertex.x, rect.mMinPosition.x, rect.mMaxPosition.x, pos_x_min, pos_x_max) * scale.x;
				float pos_y = nap::math::fit<float>(vertex.y, rect.mMinPosition.y, rect.mMaxPosition.y, pos_y_min, pos_y_max) * scale.y;
				positions.emplace_back(glm::vec3(pos_x, pos_y, 0.0f));

				// Store previous point
				previous_point = &vertex;
			}

			// If the shape is closed but the first and end points are the same, we can discard the last point
			assert(positions.size() > 0);
			bool is_closed = states[path_index];
			if (is_closed && glm::distance(positions.front(), positions.back()) <= math::epsilon<float>())
			{
				positions.pop_back();
				uvs.pop_back();
			}

			// Make sure there's enough vertices to create a segment
			if (!errorState.check(positions.size() >= 2, "not enough unique vertices in line from file: %s", mFile.c_str()))
				return false;

			// Now we have the final vertex positions of this line we can calculate their respective normals
			if (positions.size() > 1)
			{
				// The normal to rotage against 
				glm::vec3 crossn(0.0f, 0.0f, -1.0f);
				
				// Resize normal array to be size of vertices
				normals.resize(positions.size());

				// Calculate the normal based on the previous and next vertex
				for (int i = 1; i < positions.size() - 1; i++)
				{
					// Get vector pointing to next and previous vertex
					glm::vec3 dnormal_one = glm::normalize(positions[i + 1] - positions[i]);
					glm::vec3 dnormal_two = glm::normalize(positions[i] - positions[i - 1]);

					// Rotate around z using cross product
					normals[i] = glm::cross(glm::normalize(math::lerp<glm::vec3>(dnormal_one, dnormal_two, 0.5f)), crossn);
				}

				// Closed shapes need to wrap the first and last vertex
				if (is_closed)
				{
					// First normal
					glm::vec3 dnormal_one = glm::normalize(positions[1] - positions.front());
					glm::vec3 dnormal_two = glm::normalize(positions.front() - positions.back());
					normals[0] = glm::cross(math::lerp<glm::vec3>(dnormal_one, dnormal_two, 0.5f), crossn);
					
					// Last normal
					dnormal_one = glm::normalize(positions.front() - positions.back());
					dnormal_two = glm::normalize(positions.back() - positions[positions.size() - 2]);

					normals.back() = glm::cross(math::lerp<glm::vec3>(dnormal_one, dnormal_two, 0.5f), crossn);
				}
				// Otherwise the first vertex uses the next position and the last one the previous
				else
				{
					normals[0] = glm::cross(glm::normalize(positions[1] - positions.front()), crossn);
					normals.back() = normals[normals.size() - 2];
				}
			}
			else
			{
				normals.front() = { 0,1.0,0.0 };
			}

			addShape(is_closed, positions, normals, uvs);
			mClosedStates.emplace_back(is_closed);
		}

		mMeshInstance->setDrawMode(EDrawMode::LineStrip);
		return mMeshInstance->init(errorState);
	}


	void LineFromFile::addShape(bool closed, std::vector<glm::vec3>& pathVertices, std::vector<glm::vec3>& pathNormals, std::vector<glm::vec3>& pathUvs)
	{		
		Vec3VertexAttribute& pos_attr = mMeshInstance->getAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		Vec3VertexAttribute& uvs_attr = mMeshInstance->getAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		Vec4VertexAttribute& col_attr = mMeshInstance->getAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));
		Vec3VertexAttribute& nor_attr = mMeshInstance->getAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());

		int vertex_count = static_cast<int>(pathVertices.size());
		// Set position buffer
		pos_attr.addData(pathVertices.data(), pathVertices.size());

		// Set color buffer
		std::vector<glm::vec4> vert_colors(vertex_count, mLineProperties.mColor);
		col_attr.addData(vert_colors.data(), vert_colors.size());

		// Set normal buffer (todo implement)
		nor_attr.addData(pathNormals.data(), pathNormals.size());

		// Set uv buffer (todo implement)
		uvs_attr.addData(pathUvs.data(), pathUvs.size());

		MeshShape& shape = mMeshInstance->createShape();
		utility::generateIndices(shape, vertex_count, mMeshInstance->getNumVertices());
		mMeshInstance->setNumVertices(mMeshInstance->getNumVertices() + vertex_count);
	}

}