#pragma once

// Local Includes
#include "utility/errorstate.h"
#include "rtti/typeinfo.h"

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
		/*
		* Locate project data directory either next to our binary or with project source.
		* @param errorState If false is returned, contains information about the error.
		* @param unnamedNapkinFlag If true we presume we're being used in a non-project context and use a placeholder
	    *        data path (the executable directory) instead of going searching
		* @return True is data path found, false otherwise.
		*/
		// TODO change approach or name unnamedNapkinFlag
		bool populatePath(utility::ErrorState& errorState, bool unnamedNapkinFlag);
		
		/*
		 * Returns the project's data path.
		 * @return The data path.
		 */
		std::string getDataPath();

	private:
		std::string							mDataPath;						// Our found project data path
		
	};
}
