// Local Includes
#include "audiotestapp.h"

// External Includes
#include <nap/logger.h>

// Std includes
#include <thread>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AudioTestApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	bool AudioTestApp::init(utility::ErrorState& error)
	{
		// Load scene
		nap::ResourceManager* resourceManager = getCore().getResourceManager();
		if (!resourceManager->loadFile(mJsonFile, error))
		{
			error.fail("Unable to deserialize resources: \n %s", error.toString().c_str());
			return false;
		}
		return true;
	}


	void AudioTestApp::update(double deltaTime)
	{
	}


	void AudioTestApp::shutdown()
	{

	}
}