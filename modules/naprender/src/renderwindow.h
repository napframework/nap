#pragma once

// Local Includes
#include "renderservice.h"

// External Includes
#include <nap/windowresource.h>
#include <utility/dllexport.h>
#include <rect.h>

namespace nap
{
	/**
	 * 3D render window resource that can be declared in json.
	 * This resource offers an interface to change window settings and manages
	 * an OpenGL window including associated render context.
	 * It is important to activate the window before issuing any draw commands!
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
		 *	@return the width of the window in pixels
		 */
		int getWidth() const													{ return mWindow->getSize().x; }

		/**
		 *	@return the height of the window in pixels
		 */
		int getHeight() const													{ return mWindow->getSize().y; }

		/**
		 *	@return the window title
		 */
		const std::string& getTitle() const										{ return mTitle; }

		/**
		 *	@return if the window is resizable
		 */
		bool isResizable() const												{ return mResizable; }
	
		/**
		 * Turns full screen on / off
		 * This is the windowed full screen mode, game is not supported
		 * @param value if the window is set to fill the screen or not
		 */
		void setFullscreen(bool value);

		/**
		 * Toggles full screen on / off
		 */
		void toggleFullscreen();

		/**
		 * Sets the width of the window
		 * @param width the new width of the window in pixels
		 */
		void setWidth(int width);

		/**
		 * Sets the height of the window
		 * @param height the new window height in pixels
		 */
		void setHeight(int height);

		/**
		 * Sets the position of the window on screen
		 * @param position the new screen position in pixel coordinates
		 */
		void setPosition(const glm::ivec2& position);

		/**
		 * @return the window position in pixel coordinates
		 */
		const glm::ivec2 getPosition() const;

		/**
		 *	@return the hardware window number
		 */
		virtual uint getNumber() const override;

		/**
		 * @return the window as a rectangle
		 */
		math::Rect getRect() const;

		/**
		* The back buffer for an OpenGL window isn't an actual frame buffer
		* but allows for handling windows and render targets inside the framework
		* in a similar way. Associating a back buffer with a window also ensures, in this case,
		* that the opengl viewport always matches the window dimensions
		* @return the back buffer associated with this window
		*/
		const opengl::BackbufferRenderTarget& getBackbuffer() const;

		/**
		* The back buffer for an OpenGL window isn't an actual frame buffer
		* but allows for handling windows and render targets inside the framework
		* in a similar way. Associating a back buffer with a window also ensures, in this case,
		* that the opengl viewport always matches the window dimensions
		* @return the back buffer associated with this window
		*/
		opengl::BackbufferRenderTarget& getBackbuffer();

	private:
		void handleEvent(const Event& event);

	public:
		int										mWidth			= 512;							///< Property: 'Width' of the window in pixels
		int										mHeight			= 512;							///< Property: 'Height' of the window in pixels
		bool									mBorderless		= false;						///< Property: 'Borderless' if the window has any borders
		bool									mResizable		= true;							///< Property: 'Resizable' if the window is resizable
		bool									mSync			= true;							///< Property: 'Sync' If v-sync is enabled
		std::string								mTitle			= "";							///< Property: 'Title' window title
		glm::vec4								mClearColor		= { 0.0f, 0.0f, 0.0f, 1.0f };	///< Property: 'ClearColor' background clear color

	private:
		RenderService*							mRenderService	= nullptr;						// Render service
		std::shared_ptr<GLWindow>				mWindow			= nullptr;						// Actual OpenGL hardware window
		bool									mFullscreen		= false;						// If the window is full screen or not
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
