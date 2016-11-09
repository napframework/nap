// Local Includes
#include "napbrowsercomponent.h"
#include "napawesomiumservice.h"

// External Includes
#include <utils/nofUtils.h>
#include <napoftransform.h>
#include <napofsimpleshapecomponent.h>
#include <Utils/nofUtils.h>
#include <Awesomium/JSValue.h>

namespace nap
{
	// Constructor
	BrowserComponent::BrowserComponent()
	{
		mResolution.connectToValue(mDimChanged);
		mURL.connectToValue(mURLChanged);
		mTransparent.connectToValue(mTransparencyChanged);
		mJavaScriptObject.connectToValue(mJavascriptObjectChanged);
		mReload.connectToValue(mReloadChanged);
	}

	// Destructor
	BrowserComponent::~BrowserComponent()
	{
		destroyBrowser();
	}


	// Creates a new webview (browser) in Awesomium. Also allocates the frame buffer image and texture
	void BrowserComponent::Setup()
	{
		// Destroy the currently active browser
		destroyBrowser();

		// Create new web view through service
		assert(mService != nullptr);
		AwesomiumService* aw_service = static_cast<AwesomiumService*>(mService);
		mWebView = aw_service->getCore().CreateWebView(mResolution.getValue().x, mResolution.getValue().y);

		// Register listener
		mWebView->set_load_listener(this);

		// Register javascript listener
		mWebView->set_js_method_handler(this);

		// Set transparency
		mWebView->SetTransparent(mTransparent.getValue());

		// Allocate frame buffer
		int width  = mResolution.getValue().x;
		int height = mResolution.getValue().y;
		mFrame.allocate(width, height, OF_IMAGE_COLOR_ALPHA);

		// Set texture pointer
		mTexture.setValue(&mFrame.getTexture());

		// If there's a javascript object we need to bind, do it
		if (mJavaScriptObject.getValue().isValid())
			bindJSObject(mJavaScriptObject.getValue());

		// If there's an url, load it
		if (mURL.getValue() != "")
			loadUrl(mURL.getValue());
	}


	// URL Changed
	void BrowserComponent::loadUrl(const std::string& inValue)
	{
		// Default web page prefixes
		const static std::array<std::string, 2> sWebPaths = { "www", "http" };
		
		// Make sure we have a web view
		if (mWebView == nullptr)
		{
			Logger::warn("can't load url: " + inValue + " ,no webview");
			return;
		}

		// Check if it's a web or local url
		std::string c_url;
		if (gStartsWith(inValue, sWebPaths[0]) || gStartsWith(inValue, sWebPaths[1]))
			c_url = inValue;
		else
			c_url = AwesomiumService::sOFAddressToLocalAddress(inValue);

		// Convert to local address
		Awesomium::WebURL web_url = Awesomium::WebURL(Awesomium::WSLit(c_url.c_str()));
		if (!web_url.IsValid())
		{
			Logger::fatal("invalid url: " + inValue);
			return;
		}
	
		// Load and give focus
		mWebView->LoadURL(web_url);
		mWebView->Focus();
	}


	// Executes some javascript
	Awesomium::JSValue BrowserComponent::doJavaScript(std::string inJavaScript)
	{
        Logger::debug("Sending js: " + inJavaScript);
		auto result = mWebView->ExecuteJavascriptWithResult(Awesomium::WSLit(inJavaScript.c_str()), Awesomium::WSLit(""));
		if (mWebView->last_error())
            Logger::warn("Javascript call failed: %s", inJavaScript.c_str());
		return result;
	}


	// Update component
	void BrowserComponent::Update()
	{
		// Make sure webview is available
		if (mWebView == nullptr)
		{
			Logger::warn("Invalid web view: " + this->getName());
			return;
		}

		// Sample view as surface
		mWebSurface = (Awesomium::BitmapSurface*)mWebView->surface();
		if (mWebSurface == nullptr)
			return;

		// Copy buffer
		if (mWebSurface->buffer() != nullptr && mWebSurface->is_dirty())
		{
			mWebSurface->CopyTo(mFrame.getPixels(), mFrame.getWidth() * 4, 4, true, false);
			mFrame.update();
		}
	}


	// Called when the frame starts to load, only take in to account main frame
	void BrowserComponent::OnBeginLoadingFrame(Awesomium::WebView* caller, int64 frame_id, bool is_main_frame, const Awesomium::WebURL& url, bool is_error_page)
	{
		if (!is_main_frame)
			return;

		Logger::debug(getName() + ": Started loading web page: " + mURL.getValue());
		mState = State::Loading;

		// Signal start
		loadStarted(*this);
	}


	void BrowserComponent::OnFailLoadingFrame(Awesomium::WebView* caller, int64 frame_id, bool is_main_frame, const Awesomium::WebURL& url, int error_code, const Awesomium::WebString& error_desc)
	{
		if (!is_main_frame)
		{
			Logger::warn(getName() + "Failed loading element from web page: " + mURL.getValue());
			return;
		}

		// Log that we failed to load an element from the page
		Logger::warn(getName() + ": Failed loading web page: " + mURL.getValue());
		mState = State::Failed;

		// Trigger load
		loadFinished(*this);
	}


	void BrowserComponent::OnFinishLoadingFrame(Awesomium::WebView* caller, int64 frame_id, bool is_main_frame, const Awesomium::WebURL& url)
	{
		if (!is_main_frame)
			return;

		// Only change state if frame loaded successfully
		mState = mState == State::Failed ? mState : State::Success;
		Logger::debug(getName() + ": Finished loading web page: " + mURL.getValue());
		loadFinished(*this);
	}


	// Occurs when the page has loaded but is not displayed, javascript functionality is now available
	void BrowserComponent::OnDocumentReady(Awesomium::WebView* caller, const Awesomium::WebURL& url)
	{
		const std::string& sizeDivID = mAdjustSizeToDiv.getValueRef();
		if (!sizeDivID.empty())
			adjustSizeToDiv(sizeDivID);


		documentReady(*this);
	}


	// Destroys the currently bound web browser and resets the state
	void BrowserComponent::destroyBrowser()
	{
		if (mWebView)
		{
			mWebView->Destroy();
			mWebView = nullptr;
		}
		mState = State::Idle;
	}


	// Reloads the web page
	void BrowserComponent::reload(bool ignoreCache)
	{
		if (mWebView == nullptr)
		{
			Logger::warn("no active web-view loaded");
			return;
		}

		if (mURL.getValue() == "")
		{
			Logger::warn("no url specified");
			return;
		}

		mWebView->Reload(ignoreCache);
	}


	// Occurs when transparency changes
	void BrowserComponent::transparencyChanged(const bool& inValue)
	{
		if (mWebView == nullptr)
			return;
		mWebView->SetTransparent(inValue);
	}


	void BrowserComponent::adjustSizeToDiv(const std::string& divID)
	{
		using namespace Awesomium;

		JSValue retVal = mWebView->ExecuteJavascriptWithResult(WSLit("getAppSize();"), WSLit(""));
		if (!retVal.IsArray()) return;
		auto arr = retVal.ToArray();
		
		JSValue width = arr[0];
		if (!width.IsNumber()) return;
		int w = width.ToInteger();

		JSValue height = arr[1];
		if (!height.IsNumber()) return;

		int h = height.ToInteger();

		mResolution.setValue({ w, h });

		//mWebView->Reload(true);
	}


	// Occurs when the browser received a clicked signal
	void BrowserComponent::pointerClicked(PointerPressEvent& inEvent)
	{
		// Don't allow multi touch when working on mouse injection
		mTouchId = inEvent.mId;

		// Store touch location
		mTouchPos = ofVec2f(inEvent.mX, inEvent.mY);

		mWebView->InjectMouseDown((Awesomium::MouseButton)inEvent.mButton.getValue());
	}


	// Occurs when the browser received a drag signal
	void BrowserComponent::pointerDragged(PointerDragEvent& inEvent)
	{
		// Don't allow multiple drags
		if (inEvent.mId != mTouchId)
			return;

		// Only allow drags over certain distance
		if (mTouchPos.distance(ofVec2f(inEvent.mX, inEvent.mY)) < 2.0f)
			return;

		// Inject mouse move
		ofVec2i coords;
		if (!getBrowserCoordinates(inEvent, coords))
			return;
		mWebView->InjectMouseMove(coords.x, coords.y);
	}

	
	// Occurs when the browser received a mouse released signal
	void BrowserComponent::pointerReleased(PointerReleaseEvent& inEvent)
	{
		if (inEvent.mId != mTouchId)
			return;

		mWebView->InjectMouseUp((Awesomium::MouseButton)inEvent.mButton.getValue());
	}


	// Occurs when mouse is moved within the view
	void BrowserComponent::pointerMoved(PointerMoveEvent& inEvent)
	{
		ofVec2i coords;
		if (!getBrowserCoordinates(inEvent, coords))
			return;

		mWebView->InjectMouseMove(coords.x, coords.y);
	}


	// Returns browser coordinates based on the input event, tries to use world space coordinates
	bool BrowserComponent::getBrowserCoordinates(PointerEvent& inEvent, ofVec2i& outCoordinates)
	{
		// Store 
		outCoordinates.x = inEvent.mX.getValue();
		outCoordinates.y = inEvent.mY.getValue();

		nap::AttributeBase* world_base_attr = inEvent.getAttribute("worldpos");
		if (world_base_attr == nullptr)
			return false;

		// Cast with safety check
		if (!world_base_attr->getTypeInfo().isKindOf(RTTI_OF(Attribute<ofVec3f>)))
		{
			nap::Logger::warn("world space attribute is not of kind: ofVec3f");
			return false;
		}
		Attribute<ofVec3f>* world_pos_attr = static_cast<Attribute<ofVec3f>*>(world_base_attr);
		
		// Fetch additional components for computing relative input bounds
		nap::OFTransform* xform = this->getParent()->getComponent<nap::OFTransform>();
		nap::OFPlaneComponent* plane = this->getParent()->getComponent<nap::OFPlaneComponent>();
		
		if (xform == nullptr || plane == nullptr)
			return false;

		// Get bounds
		ofVec3f min_bounds, max_bounds;
		plane->getBounds(min_bounds, max_bounds);

		// Get world space bounds
		min_bounds = min_bounds * xform->getGlobalTransform();
		max_bounds = max_bounds * xform->getGlobalTransform();

		// Check if it's inside
		ofRectangle rect(min_bounds, max_bounds);
		if (!rect.inside(world_pos_attr->getValue().x, world_pos_attr->getValue().y))
			return false;

		// Set new coordinates
		outCoordinates.x = (int)gFit(world_pos_attr->getValue().x, min_bounds.x, max_bounds.x, 0, mResolution.getValue().x);
		outCoordinates.y = (int)gFit(world_pos_attr->getValue().y, min_bounds.y, max_bounds.y, mResolution.getValue().y, 0);

		return true;
	}


	// Creates and binds a Javascript callable object
	// This makes sure that any callback defined in @JavaScriptCallable is bound
	void BrowserComponent::bindJSObject(const JavaScriptCallable& inValue)
	{
		if (mWebView == nullptr)
		{
			Logger::warn("can't bind javascript object, browser not yet available: %s", this->getName());
			return;
		}

		if (!inValue.isValid())
		{
			Logger::warn("%s: can't bind un-named callable javascript object!", inValue.object.c_str());
		}

		// Ensure the variable doesn't exist already (is bound)
		Awesomium::JSValue js_value = mWebView->ExecuteJavascriptWithResult(Awesomium::WSLit(inValue.object.c_str()), Awesomium::WSLit(""));
		if (!js_value.IsUndefined())
		{
			if (!js_value.IsObject())
			{
				Logger::warn("item with name: %s is not a javascript object but is defined");
				return;
			}
		}
		else
		{
			js_value = mWebView->CreateGlobalJavascriptObject(Awesomium::WSLit(inValue.object.c_str()));
			if (js_value.IsUndefined())
			{
				Logger::warn("%s: Unable to create javascript object with name: %s", this->getName().c_str(), inValue.object.c_str());
				return;
			}
		}

		// Copy object (ref counted)
		inValue.mObject = js_value.ToObject();

		// Don't bind if callback is empty or method already exists and is bound
		if (inValue.callback.empty() || inValue.mObject.HasMethod(Awesomium::WSLit(inValue.callback.c_str())))
			return;

		// Bind callbacks
		inValue.mObject.SetCustomMethod(Awesomium::WSLit(inValue.callback.c_str()), inValue.returnvalue);
	}


	// When the browser invokes a method this component listens to
	void BrowserComponent::OnMethodCall(Awesomium::WebView* caller, unsigned int remote_object_id, const Awesomium::WebString& method_name, const Awesomium::JSArray& args)
	{
		if (caller != this->mWebView)
		{
			nap::Logger::warn("unbound webview: %s received javascript callback", this->getName().c_str());
			return;
		}
		
		// Let derived classes handle the call
		onCalled(Awesomium::ToString(method_name), args);
	}


	// When the browser invokes a method with return value this browser listens to
	Awesomium::JSValue BrowserComponent::OnMethodCallWithReturnValue(Awesomium::WebView* caller, unsigned int remote_object_id, const Awesomium::WebString& method_name, const Awesomium::JSArray& args)
	{
		Awesomium::JSValue v;

		if (caller != this->mWebView)
		{
			nap::Logger::warn("unbound webview: %s received javascript callback with return value", this->getName().c_str());
			return v;
		}

		// TODO -> FORWARD
		return v;
	}
}

RTTI_DEFINE(nap::BrowserComponent)