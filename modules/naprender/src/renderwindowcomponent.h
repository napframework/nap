#pragma once

// Local Includes
#include "renderservice.h"
#include "renderattributes.h"

// External Includes
#include <nap.h>
#include "nap/windowresource.h"

namespace nap
{
	class RenderWindowResource : public WindowResource
	{
		RTTI_ENABLE(WindowResource)

	public:
		friend class RenderService;

		// Default constructor
		RenderWindowResource() = default;
		~RenderWindowResource();

		RenderWindowResource(RenderService& renderService);

		/**
		* Creates internal texture resource.
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
		RenderWindow* getWindow()												{ return mWindow.get(); }

		/**
		 * Swaps window buffers
		 */
		void swap() const														{ mWindow->swap(); }

		/**
		 * Makes this window active
		 * calls activate afterwards
		 */
		void makeActive()														{ mWindow->makeCurrent(); }

	private:
		void handleEvent(const Event& event);

	public:
		int								mWidth			= 512;			// Width of the window
		int								mHeight			= 512;			// Height of the window
		bool							mBorderless		= false;		// If the window is borderless
		bool							mResizable		= true;			// If the window is resizable
		std::string						mTitle;							// Name of the window

	private:
		RenderService*					mRenderService	= nullptr;
		std::unique_ptr<RenderWindow>	mWindow			= nullptr;
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
			return RTTI_OF(RenderWindowResource);
		}

		/**
		* @return Creates a WindowResource
		*/
		virtual rtti::RTTIObject* create() override
		{
			return new RenderWindowResource(mRenderService);
		}

	private:
		RenderService& mRenderService;
	};


	/**
	 * Render window. 
	 * When adding this object to an entity a new render window is created
	 * If you want to change the window settings on construction don't add
	 * it immediately, create this component without adding it, change the
	 * window settings and attach to it's parent. The render service
	 * will handle all opengl related initialization calls and creates
	 * the actual window. The window is managed by this component and 
	 * destroyed upon removal Every window is associated
	 * with it's own drawing context. Note that the window this object
	 * manages can be of any type, ie: opengl, direct3d etc.
	 */
	class RenderWindowComponent : public ServiceableComponent
	{
		friend class RenderService;
		RTTI_ENABLE(ServiceableComponent)
	public:
		/**
		 * Constructor
		 */
		RenderWindowComponent();

		/**
		 * By default the window is not visible
		 * By making the window visible for the first time the window
		 * is constructed
		 */
		SignalAttribute show		{ this, "Show" };

		/**
		 * Hides the window
		 */
		SignalAttribute hide		{ this, "Hide" };

		/**
		 * Connect to this signal if you want to know when this window
		 * is made active. Subsequent render calls will be associated
		 * with this window after activate has been triggered
		 */
		SignalAttribute activate		{ this, "setActive" };

		/**
		* These attributes only work when the window has
		* been registered with the render service
		*/
		Attribute<glm::ivec2> position	{ this, "position",{ 256, 256 } };
		Attribute<glm::ivec2> size		{ this, "size",{ 512, 512 } };
		Attribute<std::string> title	{ this, "title", "RenderWindow" };
		Attribute<bool> sync			{ this, "sync", false };
		Attribute<bool> fullScreen		{ this, "fullScreen", false };

		/**
		 * @return if the component manages a window. 
		 * If show hasn't been called this call will resolve to false
		 */
		bool hasWindow() const													{ return mWindow != nullptr; }

		/**
		 * @return the window managed by this component
		 */
		RenderWindow* getWindow()												{ return mWindow.get(); }

		/**
		 * Swaps window buffers
		 */
		void swap() const														{ mWindow->swap(); }

		/**
		 * Makes this window active
		 * calls activate afterwards
		 */
		void makeActive();

		/**
		 * Sets window construction settings
		 * These settings are used when the window is constructed
		 */
		void setConstructionSettings(const RenderWindowSettings& settings);

		/**
		 * Returns window construction settings
		 */
		const RenderWindowSettings& getConstructionSettings() const				{ return mSettings; }

		/**
		* @return time it took in seconds to compute last frame
		*/
		double getDeltaTime() const;

		/**
		* @return time it took in seconds to compute last frame
		*/
		float getDeltaTimeFloat() const;

		/**
		* @return frames per seconds
		*/
		float getFps() const;

	protected:

		/*
		* Hides / shows the window
		*/
		void onShowWindow(const SignalAttribute& signal);
		void onHideWindow(const SignalAttribute& signal);

		/**
		 * Attribute changes
		 */
		void onTitleChanged(AttributeBase& title);
		void onPositionChanged(AttributeBase& position);
		void onSizeChanged(AttributeBase& size);
		void onSyncChanged(AttributeBase& value);
		void onFullscreenChanged(AttributeBase& value);

		/**
		 * Occurs when the window is registered with the render service
		 * This call is necessary because of opengl initialization order
		 * the service will provide this component with a window and valid opengl context
		 * when on construction attributes are changed the window might not be available
		 * this call makes sure that the attributes are pushed and all signals are connected
		 */
		virtual void registered() override;

		// Slot declarations
		NSLOT(showWindow, const SignalAttribute&, onShowWindow)
		NSLOT(hideWindow, const SignalAttribute&, onHideWindow)
		NSLOT(titleChanged, AttributeBase&, onTitleChanged)
		NSLOT(positionChanged, AttributeBase&, onPositionChanged)
		NSLOT(sizeChanged, AttributeBase&, onSizeChanged)
		NSLOT(syncChanged, AttributeBase&, onSyncChanged)
		NSLOT(fullScreenChanged, AttributeBase&, onFullscreenChanged)

	private:
		/**
		 * Pointer to the window that this component manages
		 * This object is set when the component is registered with the
		 * render service
		 */
		std::unique_ptr<RenderWindow> mWindow = nullptr;

		// Settings used when constructing the window
		RenderWindowSettings mSettings;

		/**
		* Holds the current frame time
		*/
		NanoSeconds	mDeltaTime;				//< Frame render time in nanoseconds
		TimePoint	mFrameTimeStamp;		//< Last recorded frame time

		/**
		 * Holds fps related values
		 */									// Fps specific members
		double 		mFpsTime = 0.0;			//< Fps specific counter
		float		mFps = 0.0f;			//< Current number of frames per second
		uint32		mFrames = 0;			//< Frame counter

		/**
		 * Window draw call, only accessible by RenderService
		 * Updates time related values and calls the draw signal
		 */
		virtual void doDraw();


		/**
		 * Window update call, only accesible by RenderService
		 * Triggers update signal
		 */
		virtual void doUpdate();

		/**
		* Updates the fps counter
		* @param deltaTime the time between the two frames
		*/
		void updateFpsCounter(double deltaTime);

	};
}
