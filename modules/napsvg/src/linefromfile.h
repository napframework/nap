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
		PX = 0,				// Pixels
		PT,					// Points
		PC,					// Points Centimeter
		MM,					// Millimeter
		CM,					// Centimeter
		DPI					// Dots per inch
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

		/**
		 *	@return the line read from file
		 */
		virtual MeshInstance& getMeshInstance() override;

		/**
		 *	@return the line read from file
		 */
		virtual const MeshInstance&	getMeshInstance() const override;

		/**
		 *	@return the total number of lines associated with the svg file
		 */
		int getCount() const													{ return mLineInstances.size(); }

		/**
		 * Set the index of the currently active line, ie: the one that is return when called @getMeshInstance
		 * @param index the new active line
		 */
		void setLineIndex(int index);

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

		// Holds all the extracted line instances
		std::vector<std::unique_ptr<MeshInstance>> mLineInstances;

		// Utility for extracting lines from all the paths
		bool extractLinesFromPaths(const SVGPaths& paths, const SVGState& states, const math::Rect& rectangle, utility::ErrorState& error);

		// Create a mesh instance out of curve sampled vertices
		bool initLineFromPath(MeshInstance& line, std::vector<glm::vec3>& pathVertices, std::vector<glm::vec3>& pathNormals, std::vector<glm::vec3>& pathUvs, bool isClosed, utility::ErrorState& error);
	};
}