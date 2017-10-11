// main.cpp : Defines the entry point for the console application.
//
// Local Includes
#include "apprunner.h"

// Nap includes
#include <nap/core.h>

// predefines
void run(nap::Core& core, std::unique_ptr<nap::AppRunner>& appRunner);

// Main loop
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;
	
	std::unique_ptr<nap::AppRunner> appRunner = std::make_unique<nap::AppRunner>();
	
	// Initialize render stuff
	if (!appRunner->init(core))
		return -1;
	
	// Run application
	run(core, appRunner);
	
	return 0;
}


void run(nap::Core& core, std::unique_ptr<nap::AppRunner>& appRunner)
{
	// Run function
	bool loop = true;
	
	// Loop
	while (loop)
	{
		opengl::Event event;
		if (opengl::pollEvent(event))
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
					
					if (press_event->mKey == nap::EKeyCode::KEY_f)
					{
						static bool fullscreen = true;
						appRunner->setWindowFullscreen("Window", fullscreen);
						fullscreen = !fullscreen;
					}
				}
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
		appRunner->update();
		
		// Render
		appRunner->render();
	}
	
	// Shutdown
	appRunner->shutdown();
	
	// Delete AppRunner now so that its entities etc are cleaned up before ObjectPtrManager destruction
	appRunner.reset();
}
     
