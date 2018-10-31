#pragma once

// Local Includes
#include "app.h"

// External Includes
#include <rtti/typeinfo.h>
#include <sdleventconverter.h>

namespace nap
{
	// Forward Declares
	class IMGuiService;

	/**
	 * Helper object that allows for custom app processing behavior inside the AppRunner update loop
	 */
	class NAPAPI BaseAppEventHandler
	{
		RTTI_ENABLE()
	public:
		/**
		 * Constructor
		 * @param app the app this event handler works with when processing messages
		 */
		BaseAppEventHandler(BaseApp& app);

		// Default Destructor
		virtual ~BaseAppEventHandler() = default;

		/**
		 * Copy is not allowed
		 */
		BaseAppEventHandler(BaseAppEventHandler&) = delete;
		BaseAppEventHandler& operator=(const BaseAppEventHandler&) = delete;

		/**
		 * Move is not allowed
		 */
		BaseAppEventHandler(BaseAppEventHandler&&) = delete;
		BaseAppEventHandler& operator=(BaseAppEventHandler&&) = delete;

		/**
		 * Called before running the app loop but after all services have been initialized
		 * Use this call to initialize functionality before starting the process loop
		 */
		virtual void start() 				{ }

		/**
		 * This is called within the main loop of the app runner to specify
		 * specific event process behavior. For example, when working with a window
		 * you might want to check for window or input messages and forward those to the application
		 * This function is invoked at the beginning of the app loop
		 */
		virtual void process()				{ }

		/**
		 * Called before shutting down all services and exiting the app
		 */
		virtual void shutdown()				{ }

		/**
		 *	Returns the application as an object of type T
		 */
		template<typename T>
		T& getApp();

	protected:
		BaseApp& mApp;						// The app associated with this app event handler
	};


	/**
	 * Default application event handler.
	 * Forwards key, mouse and window events to the running application
	 */
	class NAPAPI AppEventHandler : public BaseAppEventHandler
	{
		RTTI_ENABLE(BaseAppEventHandler)
	public:
		AppEventHandler(App& app);

		/**
		 * This call creates the SDL Input Converter
		 */
		virtual void start() override;

		/**
		 * This call polls SDL for various messages, translates those messages
		 * in nap events and forwards those to the default nap application
		 */
		virtual void process() override;

		/**
		 * This call deletes the input converter
		 */
		virtual void shutdown() override;

	private:
		std::unique_ptr<SDLEventConverter> mEventConverter = nullptr;
	};


	/**
	 * Application event handler that is designed to work with applications that host a graphical user interface.
	 * This class checks if the user is interacting with a GUI element, if so, 
	 * no input events are forwarded to the application.
	 */
	class NAPAPI GUIAppEventHandler : public BaseAppEventHandler
	{
		RTTI_ENABLE(BaseAppEventHandler)
	public:
		GUIAppEventHandler(App& app);

		/**
		 * This call creates the SDL Input Converter
		 */
		virtual void start() override;

		/**
		 * This call polls the various SDL messages and filters them
		 * based on GUI activity. If the gui is actively used the events
		 * are not forwarded to the running app
		 */
		virtual void process() override;

		/**
		 * This call deletes the input converter
		 */
		virtual void shutdown() override;

	private:
		std::unique_ptr<SDLEventConverter> mEventConverter = nullptr;
		IMGuiService* mGuiService = nullptr;
	};


	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T& nap::BaseAppEventHandler::getApp()
	{
		assert(mApp.get_type().is_derived_from(RTTI_OF(T)));
		return static_cast<T&>(mApp);
	}
}