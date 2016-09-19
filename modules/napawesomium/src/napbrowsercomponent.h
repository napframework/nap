#pragma once

// Nap Includes
#include <nap/serviceablecomponent.h>
#include <nap/signalslot.h>

// Std Includes
#include <string.h>

// Awesomium Includes
#include <Awesomium/BitmapSurface.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/WebCore.h>
#include <Awesomium/WebViewListener.h>
#include <Awesomium/JSObject.h>

// OF Includes
#include <ofImage.h>
#include <napofattributes.h>
#include <Utils/ofVec2i.h>

// Input includes
#include <napinputcomponent.h>

// Local includes
#include <napjscriptcallable.h>

namespace nap
{
	/**
	@brief BrowserComponent
	Uses an browser view to render to a texture
	**/
	class BrowserComponent : 
		public ServiceableComponent, 
		public Awesomium::WebViewListener::Load,
		public Awesomium::JSMethodHandler
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)

		// Browse Component State
		enum class State
		{
			Success,
			Failed,
			Loading,
			Idle = -1
		};

	public:
		// Default constructor
		BrowserComponent();
		virtual ~BrowserComponent();

		// Attributes
		Attribute<std::string>			mURL				{ this, "URL", "" };
		Attribute<JavaScriptCallable>	mJavaScriptObject	{ this, "JSObject" };
		Attribute<bool>					mTransparent		{ this, "Transparent", true };
		Attribute<ofTexture*>			mTexture 			{ this, "Texture", nullptr };
		Attribute<std::string>			mAdjustSizeToDiv	{ this, "AdjustSizeToDiv", "" };
		Attribute<ofVec2i>				mResolution			{ this, "Resolution", { 1024, 1024 } };		//< Will invalidate previously generated texture
		Attribute<bool>					mUpdate				{ this, "Update", true };
		Attribute<bool>					mReload				{ this, "Reload", false };

		// Getters
		bool isLoading() const								{ return mWebView->IsLoading(); }
		State getState() const								{ mState; }

		// Reload
		void reload(bool ignoreCache = false);

		// Javascript
		Awesomium::JSValue doJavaScript(std::string inJavaScript);

		// Slots
		NSLOT(mURLChanged, const std::string&, loadUrl);
		NSLOT(mJavascriptObjectChanged, const JavaScriptCallable&, bindJSObject);
		NSLOT(mDimChanged, const ofVec2i&, dimChanged);
		NSLOT(mTransparencyChanged, const bool&, transparencyChanged);
		NSLOT(mReloadChanged, const bool&, reloadChanged);

		// Input handlers
		NSLOT(mClicked, PointerPressEvent&,		pointerClicked);
		NSLOT(mDragged, PointerDragEvent&,		pointerDragged);
		NSLOT(mReleased,PointerReleaseEvent&,	pointerReleased);
		NSLOT(mMoved,	PointerMoveEvent&,		pointerMoved);

		// Copies web browser data if needed and uploads it to the GPU
		void Update();

		//////////////////////////////////////////////////////////////////////////
		// Signals
		//////////////////////////////////////////////////////////////////////////
		Signal<BrowserComponent&> loadStarted;		//< Web page load started (main frame)
		Signal<BrowserComponent&> loadFinished;		//< Web page load finished (main frame)
		Signal<BrowserComponent&> documentReady;	//< Web page is available for JavaScript execution

		//////////////////////////////////////////////////////////////////////////
		// Awesomium Webview Listener Overrides
		//////////////////////////////////////////////////////////////////////////
		
		/// This event occurs when the page begins loading a frame.
		virtual void OnBeginLoadingFrame(Awesomium::WebView* caller, int64 frame_id, bool is_main_frame,  const Awesomium::WebURL& url, bool is_error_page) override;

		/// This event occurs when a frame fails to load. See error_desc
		/// for additional information.
		virtual void OnFailLoadingFrame(Awesomium::WebView* caller, int64 frame_id, bool is_main_frame,	const Awesomium::WebURL& url, int error_code, const Awesomium::WebString& error_desc) override;

		/// This event occurs when the page finishes loading a frame.
		/// The main frame always finishes loading last for a given page load.
		virtual void OnFinishLoadingFrame(Awesomium::WebView* caller, int64 frame_id, bool is_main_frame, const Awesomium::WebURL& url) override;

		/// This event occurs when the DOM has finished parsing and the
		/// window object is available for JavaScript execution.
		virtual void OnDocumentReady(Awesomium::WebView* caller, const Awesomium::WebURL& url) override;

		//////////////////////////////////////////////////////////////////////////
		// Awesomium Javascript method handler overrides
		//////////////////////////////////////////////////////////////////////////
		
		// Called from javascript without return value
		virtual void OnMethodCall(Awesomium::WebView* caller, unsigned int remote_object_id, const Awesomium::WebString& method_name, const Awesomium::JSArray& args) override;

		// Called from javascript with return value
		virtual Awesomium::JSValue OnMethodCallWithReturnValue(Awesomium::WebView* caller, unsigned int remote_object_id, const Awesomium::WebString& method_name, const Awesomium::JSArray& args);

		//////////////////////////////////////////////////////////////////////////

	protected:

		// Virtual to be used by derived classes
		virtual void onCalled(const std::string& method_name, const Awesomium::JSArray& args) {}

		// After registration the service is available and the web view can be create
		virtual void registered() override					{ Setup(); }

	private:

		// Slots
		void loadUrl(const std::string& inValue);
		void dimChanged(const ofVec2i& inValue)				{ Setup(); }
		void transparencyChanged(const bool& inValue);
		void reloadChanged(const bool& inValue)				{ reload(); }
		void adjustSizeToDiv(const std::string& divID);
		void pointerClicked(PointerPressEvent& inEvent);
		void pointerDragged(PointerDragEvent& inEvent);
		void pointerReleased(PointerReleaseEvent& inEvent);
		void pointerMoved(PointerMoveEvent& inEvent);
		void bindJSObject(const JavaScriptCallable& inValue);

		// Creates a new WebView (browser) in Awesomium. Also allocates the component's frame buffer image and texture
		void Setup();

		// Awesomium
		Awesomium::WebView*			mWebView = nullptr;
		Awesomium::BitmapSurface*	mWebSurface = nullptr;
		ofImage						mFrame;

		// Browser state
		State						mState = State::Idle;
		
		// Input handling
		ofVec2f						mTouchPos;
		int							mTouchId = -1;

		// Destroys the browser
		void destroyBrowser();

		// Utility
		bool getBrowserCoordinates(const PointerEvent& inEvent, ofVec2i& outCoordinates);
	};
}

RTTI_DECLARE(nap::BrowserComponent)
