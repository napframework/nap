#pragma once

// Local Includes
#include <app.h>


/*
 * Select your serialized json audio demo app here:
 */
//const std::string appJson = "audiofileplay.json";
//const std::string appJson = "pythonsequencer.json";
//const std::string appJson = "ambientdrone.json";
//const std::string appJson = "midilogging.json"; // Edit the midi input device in this json file!
//const std::string appJson = "pythonosc.json";
const std::string appJson = "pythonmidi.json";


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
		virtual int shutdown() override;
        
    private:
        std::string mJsonFile = appJson;
        
	}; 
}
