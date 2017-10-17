#pragma once

// Local Includes
#include "renderservice.h"
#include "renderattributes.h"

// External Includes
#include <nap/windowresource.h>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Resource class for RenderWindow
	 */
	class NAPAPI RenderWindow : public Window
	{
		RTTI_ENABLE(Window)

	public:
		friend class RenderService;

		// Default constructor
		RenderWindow() = default;
		
		// Destructor
		virtual ~RenderWindow() override;

		/**
		 * This constructor is called when creating the render window using the resource manager
		 * Every render window needs to be aware of it's render service
		 */
		RenderWindow(RenderService& renderService);

		/**
		 * Creates window, connects to resize event.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return if the component manages a window. 
		 * If show hasn't been called this call will resolve to false
		 */
		bool hasWindow() const													{ return mWindow != nullptr; }

		/**
		 * @return the window managed by this component
		 */
		GLWindow* getWindow() const												{ return mWindow.get(); }

		/**
		 * Swaps window buffers
		 */
		void swap() const														{ mWindow->swap(); }

		/**
		 * Makes this window active, calls activate afterwards
		 */
		void makeActive()														{ mWindow->makeCurrent(); }

		/**
		 *	@return the hardware window number
		 */
		virtual uint getNumber() const override;

	private:
		void handleEvent(const Event& event);

	public:
		int										mWidth			= 512;			// Width of the window
		int										mHeight			= 512;			// Height of the window
		bool									mBorderless		= false;		// If the window is borderless
		bool									mResizable		= true;			// If the window is resizable
		bool									mSync			= true;			// If v-sync is turned on for the window
		std::string								mTitle			= "";			// Name of the window

	private:
		RenderService*							mRenderService	= nullptr;		// Render service
		std::shared_ptr<GLWindow>				mWindow			= nullptr;		// Actual OpenGL hardware window
	};


	/**
	* Factory for creating WindowResources. The factory is responsible for passing the RenderService
	* to the WindowResource on construction.
	*/
	class RenderWindowResourceCreator : public rtti::IObjectCreator
	{
	public:
		RenderWindowResourceCreator(RenderService& renderService) :
			mRenderService(renderService) { }

		/**
		* @return Type of WindowResource
		*/
		rtti::TypeInfo getTypeToCreate() const override
		{
			return RTTI_OF(RenderWindow);
		}

		/**
		* @return Creates a WindowResource
		*/
		virtual rtti::RTTIObject* create() override
		{
			return new RenderWindow(mRenderService);
		}

	private:
		RenderService& mRenderService;
	};
}
