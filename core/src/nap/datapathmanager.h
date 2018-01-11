#pragma once

// Local Includes
#include "core.h"

namespace nap
{
	
	/**
	 * Locates the project data directory either next to our binary or with project source and provides that
	 * path for use throughout.
	 * TODO: Investigate whether there's benefit of instead using a data path provided through our project.json
	 *       once integrated.
	 */
	class NAPAPI DataPathManager
	{
	public:
		/**
		* Returns the global DataPathManager.
		*/
		static DataPathManager& get();

		/*
		* Locate project data directory either next to our binary or with project source.
		* @param errorState If false is returned, contains information about the error.
		* @return True is data path found, false otherwise.
		*/
		bool populatePath(utility::ErrorState& errorState);
		
		/*
		 * Returns the project's data path.
		 * @return The data path.
		 */
		std::string getDataPath();

	private:
		std::string							mDataPath;						// Our found project data path
		
	};
}
