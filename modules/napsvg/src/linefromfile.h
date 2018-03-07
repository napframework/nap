#pragma once

#include <polyline.h>
#include <rect.h>

namespace nap
{
	/**
	 * Units associated with an svg file
	 * These units are used when loading an svg file
	 */
	enum class ESVGUnits : int
	{
		PX = 0,				///< 'px'  Pixels
		PT,					///< 'pt'  Points
		PC,					///< 'pc'  Points Centimeter
		MM,					///< 'mm'  Millimeter
		CM,					///< 'cm'  Centimeter
		DPI					///< 'dpi' Dots per inch
	};

	/**
	 * Resource that loads a set of lines from an svg file
	 * Every line is converted in a polyline. The line vertex reolution can be changed using a difference tolerance value.
	 * A lower tolerance results in less vertices. The uv's of the lines are normalized based on the
	 * total bounding box of the file, ie: all the lines in the file. The normals are perpendicular
	 * to the tangent of the line, the line itself is loaded in the x, y plane.
	 * When normalization is turned on the loaded lines will be placed relative to 0, with a default bounding box
	 * in the x, y plane of -0.5 to 0.5. Changing the scale will change these bounds.
	 */
	class NAPAPI LineFromFile : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		/**
		 *	Loads the svg file and extracts the line
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		// Property: the svg file to read
		std::string mFile;

		// Property: the cubic re-sample line tolerance
		float mTolerance = 1.0f;

		// Property: units used when reading the svg file
		ESVGUnits mUnits = ESVGUnits::PX;

		// Property: dpi used when reading the svg file
		float mDPI = 96.0f;

		// Property: if the lines should be normalized relative to 0, ie: max bounds are -0.5 and 0.5
		bool mNormalize = true;

		// Property: scale used as a multiplier for the loaded lines
		float mScale = 1.0f;

		// Property: if the lines should be flipped on the x axis
		bool mFlipX = false;

		// Property: if the lines should be flipped on the y axis
		bool mFlipY = false;

		// The currently active line
		int mLineIndex = 0;

	private:
		using SVGPaths = std::vector<std::unique_ptr<std::vector<glm::vec3>>>;
		using SVGState = std::vector<bool>;

		// Utility for extracting lines from all the paths
		bool extractLinesFromPaths(const SVGPaths& paths, const SVGState& states, const math::Rect& rectangle, utility::ErrorState& errorState);

		// Create a mesh instance out of curve sampled vertices
		void addShape(std::vector<glm::vec3>& pathVertices, std::vector<glm::vec3>& pathNormals, std::vector<glm::vec3>& pathUvs, bool isClosed);
	};
}