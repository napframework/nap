#pragma once

#include <polyline.h>

namespace nap
{
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

		// Property: the svg file to read
		std::string mFile;

		// Property: the cubic resample line tolerance
		float mTolerance = 1.0f;

	private:
		// Holds all the extracted line instances
		std::vector<std::unique_ptr<MeshInstance>> mLineInstances;

		// Create a mesh instance out of curve sampled vertices
		bool initLineFromPath(MeshInstance& line, std::vector<glm::vec3>& pathVertices, bool isClosed, utility::ErrorState& error);
	};
}