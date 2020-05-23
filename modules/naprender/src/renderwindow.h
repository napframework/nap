#pragma once

// Local Includes
#include "renderservice.h"
#include "glwindow.h"
#include "renderutils.h"

// External Includes
#include <window.h>
#include <utility/dllexport.h>
#include <rect.h>

namespace nap
{
	class Core;
	class BackbufferRenderTarget;

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

		/**
		 * The various image presentation modes.
		 * Controls the way in which images are presented to screen. 
		 */
		enum class EPresentationMode : int
		{
			Immediate,					///< The new image immediately replaces the display image, screen tearing may occur.
			Mailbox,					///< The new image replaces the one image waiting in the queue, becoming the first to be displayed. No screen tearing occurs. Could result in not-shown images, but ensures CPU does not stall.
			FIFO,						///< Based on first in first out, where every image is presented in order. No screen tearing occurs when drawing faster than monitor refresh rate. CPU could stall.
		};

		// Default constructor
		RenderWindow() = default;
		
		// Destructor
		virtual ~RenderWindow() override;

		/**
		 * This constructor is called when creating the render window using the resource manager
		 * Every render window needs to be aware of it's render service
		 */
		RenderWindow(Core& core);

		/**
		 * Creates window, connects to resize event.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the window managed by this component
		 */
		GLWindow* getWindow() const												{ return mWindow.get(); }

		/**
		 * Makes this window active, calls activate afterwards
		 */
		VkCommandBuffer makeActive()											{ return mWindow->makeCurrent(); }

		/**
         * Returns the width of the window.
         * Note that on high DPI monitors this is not the same as the pixel count.
         * To get the width in pixels use the size of the backbuffer using getWidthPixels().
		 * @return the width of the window
		 */
		int getWidth() const													{ return mWindow->getSize().x; }

        /**
         * Returns the width of this window in pixels.
         * @return the width of the window in pixels.
         */
        int getWidthPixels() const;
        
		/**
         * Returns the height of the window.
         * Note that on high DPI monitors this is not the same as the pixel count.
         * To get the height in pixels use the size of the backbuffer using getHeightPixels().
		 * @return the height of the window in pixels
		 */
		int getHeight() const													{ return mWindow->getSize().y; }
        
        /**
         * Returns the height of this window in pixels.
         * @return the width of the window in pixels.
         */
        int getHeightPixels() const;
        
		/**
		 * Shows the window and gives it input focus.
		 * This call also makes sure the window is on top of other windows.
		 */
		void show();

		/**
		 *	Hides the window
		 */
		void hide();

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
		 * Sets the width of the window.
         * When the window is drawn on a high DPI monitor the resulting pixel count of the window buffer will be higher.
		 * @param width the new width of the window in pixels
		 */
		void setWidth(int width);

		/**
         * Sets the height of the window.
         * When the window is drawn on a high DPI monitor the pixel count of the window buffer will be higher.
		 * @param height the new window height in pixels
		 */
		void setHeight(int height);

		/**
		 * Sets the window clear color.
		 * @param color the new clear color
		 */
		void setClearColor(const glm::vec4& color);

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
		 * Creates a rectangle based on the current width and height of the render window.
		 * Note that the returned dimensions of the rectangle can differ from the actual size in pixels on a high dpi monitor.
		 * To obtain a rectangle that contains the actual size of the window in pixels use: getRectPixels
		 * @return the window as a rectangle
		 */
		math::Rect getRect() const;

		/**
		 * Creates a rectangle based on the current width and height of the render window in pixels.
		 * @return the window as rectangle
		 */
		math::Rect getRectPixels() const;

		/**
		* The back buffer for an OpenGL window isn't an actual frame buffer
		* but allows for handling windows and render targets inside the framework
		* in a similar way. Associating a back buffer with a window also ensures, in this case,
		* that the opengl viewport always matches the window dimensions
		* @return the back buffer associated with this window
		*/
		const BackbufferRenderTarget& getBackbuffer() const;

		/**
		* The back buffer for an OpenGL window isn't an actual frame buffer
		* but allows for handling windows and render targets inside the framework
		* in a similar way. Associating a back buffer with a window also ensures, in this case,
		* that the opengl viewport always matches the window dimensions
		* @return the back buffer associated with this window
		*/
		BackbufferRenderTarget& getBackbuffer();

		/**
		 * Starts a render pass. Only call this when recording is enabled.
		 */
		void beginRendering();

		/**
		 * Ends a render pass. Always call this after beginRendering().
		 */
		void endRendering();

	private:
		void handleEvent(const Event& event);
		void swap() const						{ mWindow->swap(); }

	public:
		bool					mSampleShading	= true;								///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		int						mWidth			= 512;								///< Property: 'Width' of the window in pixels
		int						mHeight			= 512;								///< Property: 'Height' of the window in pixels
		bool					mBorderless		= false;							///< Property: 'Borderless' if the window has any borders
		bool					mResizable		= true;								///< Property: 'Resizable' if the window is resizable
		EPresentationMode		mMode			= EPresentationMode::Mailbox;		///< Property: 'Mode' the image presentation mode to use
		std::string				mTitle			= "";								///< Property: 'Title' window title
		glm::vec4				mClearColor		= { 0.0f, 0.0f, 0.0f, 1.0f };		///< Property: 'ClearColor' background clear color
		ERasterizationSamples	mRequestedSamples = ERasterizationSamples::Four;	///< Property: 'Samples' Controls the number of samples used during Rasterization. For even better results enable 'SampleShading'.

	private:
		RenderService*				mRenderService	= nullptr;						// Render service
		std::shared_ptr<GLWindow>	mWindow			= nullptr;						// Actual OpenGL hardware window
		bool						mFullscreen		= false;						// If the window is full screen or not
	};
}
