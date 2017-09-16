#pragma once

#include <polyline.h>

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
	 *	Resource that specifies a line read from an svg file
	 */
	class NAPAPI LineFromFile : public PolyLine
	{
		RTTI_ENABLE(PolyLine)
	public:
		/**
		 *	Loads the svg file and extracts the line
		 */
		virtual bool init(utility::ErrorState& errorState);

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
		float mFlipX = false;

		// Property: if the lines should be flipped on the y axis
		float mFlipY = false;

		// The currently active line
		int mLineIndex = 0;

	private:
		// Holds all the extracted line instances
		std::vector<std::unique_ptr<MeshInstance>> mLineInstances;

		// Utility for setting the index safely (clamped)
		void setIndex(int index);

		// Create a mesh instance out of curve sampled vertices
		bool initLineFromPath(MeshInstance& line, std::vector<glm::vec3>& pathVertices, bool isClosed, utility::ErrorState& error);
	};
}