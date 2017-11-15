#pragma once

// Local Includes
#include <app.h>


/*
 * Select your serialized json audio demo app here:
 */
//const std::string appJson = "data/audiotest/audiofileplay.json";
const std::string appJson = "data/audiotest/pythonsequencer.json";
//const std::string appJson = "data/audiotest/ambientdrone.json";
//const std::string appJson = "data/audiotest/midilogging.json"; // Edit the midi input device in this json file!


namespace nap
{
	class AudioTestApp : public BaseApp
	{
		RTTI_ENABLE(BaseApp)
        
	public:
		AudioTestApp(Core& core) : BaseApp(core)	{ }

		/**
		 *	Initialize our application specific resources
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 *	Update app resources
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Shutdown app
		 */
		virtual void shutdown() override;
        
    private:
        std::string mJsonFile = appJson;
        
	}; 
}
