// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "apprunner.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>

// predefines
void run(nap::Core& core, std::unique_ptr<nap::AppRunner>& appRunner);

// Main loop
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;
	
	// Initialize engine
	nap::utility::ErrorState error;
	if (!core.initializeEngine(error))
	{
		nap::Logger::fatal("Unable to initialize engine: %s", error.toString().c_str());
		return -1;
	}
	
	// Create app runner
	std::unique_ptr<nap::AppRunner> appRunner = std::make_unique<nap::AppRunner>();

	// Initialize app runner
	if (!appRunner->init(core, error))
		return -1;
	
	// Run application
	run(core, appRunner);
	
	return 0;
}



void run(nap::Core& core, std::unique_ptr<nap::AppRunner>& appRunner)
{
	// Run function
	bool loop = true;
	
	// Pointer to function used inside update call by core
	std::function<void(double)> update_call = std::bind(&nap::AppRunner::update, appRunner.get(), std::placeholders::_1);

	// Signal Start
	core.start();

	// Loop
	while (loop)
	{
		opengl::Event event;
		while (opengl::pollEvent(event))
		{
			// Check if we are dealing with an input event (mouse / keyboard)
			if (nap::isInputEvent(event))
			{
				nap::InputEventPtr input_event = nap::translateInputEvent(event);
				
				// If we pressed escape, quit the loop
				if (input_event->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
				{
					nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(input_event.get());
					if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
						loop = false;

				}
				
				appRunner->registerInputEvent(std::move(input_event));
			}
			
			// Check if we're dealing with a window event
			else if (nap::isWindowEvent(event))
			{
				nap::WindowEventPtr window_event = nap::translateWindowEvent(event);
				if (window_event != nullptr)
					appRunner->registerWindowEvent(std::move(window_event));
			}
		}
		
		//////////////////////////////////////////////////////////////////////////
		
		// Update
		core.update(update_call);
		
		// Render
		appRunner->render();
	}
	
	// Shutdown app
	appRunner->shutdown();
	
	// Shutdown core
	core.shutdown();

	// Delete AppRunner now so that its entities etc are cleaned up before ObjectPtrManager destruction
	appRunner.reset(); 
}




