// Local includes
#include "linefromfile.h"

// External includes
#include <nap/entity.h>
#include <nanosvg.h>
#include <utility/stringutils.h>
#include <nap/configure.h>
#include <rectangle.h>
#include <limits>

RTTI_BEGIN_ENUM(nap::ESVGUnits)
	RTTI_ENUM_VALUE(nap::ESVGUnits::PX,		"px"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::PT,		"pt"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::PC,		"pc"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::MM,		"mm"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::CM,		"cm"),
	RTTI_ENUM_VALUE(nap::ESVGUnits::DPI,	"dpi")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::LineFromFile)
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
	bool LineFromFile::init(utility::ErrorState& errorState)
	{
		// Append . before trying to load relative paths
		std::string img_path = mFile;
		if (utility::gStartsWith(img_path, "/"))
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
		std::vector<std::unique_ptr<std::vector<glm::vec3>>> extracted_paths;
		std::vector<bool> closed;

		// Rectangle that is used to compute final image bounds
		math::Rectangle svg_rect(glm::vec2(math::max<float>(), math::max<float>()), glm::vec2(math::min<float>(), math::min<float>()));

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
				closed.emplace_back(is_closed);

				// Cycle to next one
				current_path = current_path->next;
			}
			// Cycle to next shape
			current_shape = current_shape->next;
		}

		// Compute ratio when we want to normalize the image
		glm::vec2 ratio(1.0f, 1.0f);
		if (svg_rect.getWidth() < svg_rect.getHeight())
			ratio.x = svg_rect.getWidth() / svg_rect.getHeight();
		else
			ratio.y = svg_rect.getHeight() / svg_rect.getWidth();

		// Create a set of mesh instances based on those paths
		int count = 0;
		for (auto& path : extracted_paths)
		{
			// If we want to normalize the path vertices, we compute the min / max value around zero on the x and y axis, both relative to the image ratio
			if (mNormalize)
			{
				float x_min = mFlipX ? 0.5f : -0.5f;
				float x_max = mFlipX ? -0.5f : 0.5f;
				float y_min = mFlipY ? 0.5f : -0.5f;
				float y_max = mFlipY ? -0.5f : 0.5f;

				for (auto& vertex : *path)
				{
					vertex.x = nap::math::fit<float>(vertex.x, svg_rect.mMinPosition.x, svg_rect.mMaxPosition.x, x_min, x_max) * ratio.x * mScale;
					vertex.y = nap::math::fit<float>(vertex.y, svg_rect.mMinPosition.y, svg_rect.mMaxPosition.y, y_min, y_max) * ratio.y * mScale;
				}
			}
			// Otherwise we leave the vertices as is but we do want to scale them and apply the flip transformation
			// To flip a not normalized set of vertices we simply fit the vertex based on the inverse of the bounds
			else
			{
				float x_min = mFlipX ? svg_rect.mMaxPosition.x : svg_rect.mMinPosition.x;
				float x_max = mFlipX ? svg_rect.mMinPosition.x : svg_rect.mMaxPosition.x;
				float y_min = mFlipY ? svg_rect.mMaxPosition.y : svg_rect.mMinPosition.y;
				float y_max = mFlipY ? svg_rect.mMinPosition.y : svg_rect.mMaxPosition.y;

				for (auto& vertex : *path)
				{
					vertex.x = nap::math::fit<float>(vertex.x, svg_rect.mMinPosition.x, svg_rect.mMaxPosition.x, x_min, x_max) * mScale;
					vertex.y = nap::math::fit<float>(vertex.y, svg_rect.mMinPosition.y, svg_rect.mMaxPosition.y, y_min, y_max) * mScale;
				}
			}

			// Create and initialize new line based on sampled path vertices
			std::unique_ptr<MeshInstance> new_line = std::make_unique<MeshInstance>();

			if (!initLineFromPath(*new_line, *path, closed[count], errorState))
				return false;
			
			// Add line and increment count
			mLineInstances.emplace_back(std::move(new_line));
			count++;
		}

		// The the index to use
		setLineIndex(mLineIndex);

		// Delete the image
		nsvgDelete(new_image);
		return true;
	}


	nap::MeshInstance& LineFromFile::getMeshInstance()
	{
		return *(mLineInstances[mLineIndex]);
	}


	const nap::MeshInstance& LineFromFile::getMeshInstance() const
	{
		return *(mLineInstances[mLineIndex]);
	}


	void LineFromFile::setLineIndex(int index)
	{
		mLineIndex = nap::math::clamp<int>(index, 0, mLineInstances.size() - 1);
	}


	bool LineFromFile::initLineFromPath(MeshInstance& line, std::vector<glm::vec3>& pathVertices, bool closed, utility::ErrorState& error)
	{		
		PolyLine::createVertexAttributes(line);

		Vec3VertexAttribute& pos_attr = line.GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName());
		Vec3VertexAttribute& uvs_attr = line.GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetUVName(0));
		Vec4VertexAttribute& col_attr = line.GetAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0));
		Vec3VertexAttribute& nor_attr = line.GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::getNormalName());

		// re-sample line to hit x amount of vertices
		int vertex_count = math::resampleLine(pathVertices, pos_attr.getData(), mLineProperties.mVertices, closed);

		// Set color buffer
		std::vector<glm::vec4> vert_colors(vertex_count, mLineProperties.mColor);
		col_attr.setData(vert_colors);

		// Set normal buffer (todo implement)
		std::vector<glm::vec3> vert_normals(vertex_count, {0.0f, 0.0f, 0.0f});
		nor_attr.setData(vert_normals);

		// Set uv buffer (todo implement)
		std::vector<glm::vec3> vert_uvs(vertex_count, { 0.0f, 0.0f, 0.0f });
		uvs_attr.setData(vert_uvs);

		// Update
		line.setNumVertices(vertex_count);
		line.setDrawMode(closed ? opengl::EDrawMode::LINE_LOOP : opengl::EDrawMode::LINE_STRIP);

		// Initialize
		return line.init(error);
	}

}