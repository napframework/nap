#pragma once

#include <rtti/typeinfo.h>
#include <utility/errorstate.h>
#include <inputevent.h>
#include <nap/windowevent.h>
#include <nap/core.h>

namespace nap
{
	/**
	 * Acts as a blueprint for all NAP applications. This class does not receive any input events.
	 * Every application holds a reference to a Core object. The init, update, render and shutdown functions
	 * are called from within the main loop by the AppRunner.
	 */
	class NAPAPI BaseApp
	{
		RTTI_ENABLE()
	public:
		// Default Constructor
		BaseApp(nap::Core& core);

		// Default Destructor
		virtual ~BaseApp() = default;

		/**
		* Copy is not allowed
		*/
		BaseApp(BaseApp&) = delete;
		BaseApp& operator=(const BaseApp&) = delete;

		/**
		* Move is not allowed
		*/
		BaseApp(BaseApp&&) = delete;
		BaseApp& operator=(BaseApp&&) = delete;

		/**
		 * Initialize your application. Occurs after all the modules
		 * have been loaded and services have been initialized. If this call
		 * is unsuccessful the application will not continue execution
		 * @param error contains the error when initialization fails
		 * @return if initialization was successful
		 */
		virtual bool init(utility::ErrorState& error)					{ return true; }

		/**
		 * Update your application. This is called after
		 * calling update for all registered services but before render
		 * @param deltaTime the time in seconds between calls
		 */
		virtual void update(double deltaTime)							{ }

		/**
		 * Render your application. This is called after update at the end of the loop
		 */
		virtual void render()											{ }

		/**
		 * Called by the event handler when it receives an external event to quit the running application
		 * By default this calls quit() with the return value of this call, which causes the application to stop running
		 * You can override it to ignore the event or perform specific logic (such as saving a file) before the app is shutdown()
		 * @return if the application should quit, true by default
		 */
		virtual bool shutdownRequested()								{ return true; }

		/**
		 * Called when the app is shutting down after quit() has been called
		 * Use this to clear application specific data. 
		 * This is called before all the services are closed and the app exits
		 * return the application exit code, 0 (success) by default 
		 */
		virtual int shutdown()											{ return 0; }

		/**
		 * Call this from your app to terminate the application and exit the main loop.
		 */
		void quit()														{ mQuit = true; }

		/**
		 *	@return nap Core const
		 */
		const nap::Core& getCore() const								{ return mCore; }

		/**
		 * @return nap Core
		 */
		nap::Core& getCore()											{ return mCore; }

	   /**
		* @return if the application received a quit event
		*/
		bool shouldQuit() const											{ return mQuit; }

	private:
		bool mQuit = false;												// When set to true the application will exit
		nap::Core& mCore;												// Core
	};


	/**
	 * The default NAP application. Derive from this object to receive mouse, keyboard and window events.
	 * Override inputMessageReceived to receive mouse and keyboard events. Override windowMessageReceived to
	 * receive window events. This class works in conjunction with an AppEventHandler which polls and forwards incoming system messages.
	 */
	class NAPAPI App : public BaseApp
	{
		RTTI_ENABLE(BaseApp)
	public:
		App(Core& core);

		/**
		* Called when the system receives an input event
		* @param inputEvent the received input event
		*/
		virtual void inputMessageReceived(InputEventPtr inputEvent)		{ }

		/**
		* Called when the system receives a window event
		* @param windowEvent the event that contains the window information
		*/
		virtual void windowMessageReceived(WindowEventPtr windowEvent)	{ }
	};
}