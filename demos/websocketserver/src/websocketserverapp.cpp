// Local Includes
#include "websocketserverapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <scene.h>
#include <inputrouter.h>
#include <imgui/imgui.h>

// Register this application with RTTI, this is required by the AppRunner to 
// validate that this object is indeed an application
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::WebSocketServerApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and store the objects we need later on
	 */
	bool WebSocketServerApp::init(utility::ErrorState& error)
	{
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Get resource manager and load curveball json file
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("websocketserver.json", error))
			return false;

		// Extract loaded resources
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window0");
		mServerEndPoint = mResourceManager->findObject<nap::WebSocketServerEndPoint>("WebSocketServerEndPoint");

		// Extract the only scene
		ObjectPtr<Scene> scene = mResourceManager->findObject<Scene>("Scene");

		// Find the entities we're interested in
		mTextEntity = scene->findEntity("TextEntity");
		return true;
	}
	
	
	/**
	 * Updates the imgui information window
	 */
	void WebSocketServerApp::update(double deltaTime)
	{
		// Setup some gui elements to be drawn later
		ImGui::Begin("Information");
		ImGui::Text(getCurrentDateTime().toString().c_str());
		RGBAColorFloat clr = mTextHighlightColor.convert<RGBAColorFloat>();
		ImGui::TextColored(clr, "Connect a client to change the text on screen.");
		ImGui::TextColored(clr, "Use the javascript client in the 'data/websocket_html_client' directory.");
		ImGui::TextColored(clr, utility::stringFormat("Server Port: %d", mServerEndPoint->mPort).c_str());
		ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());
		if (ImGui::CollapsingHeader("Connected Clients"))
		{
			// Get all connected clients
			std::vector<std::string> host_names;
			mServerEndPoint->getHostNames(host_names);
			
			// Combine for display
			std::string msg;
			for (const auto& name : host_names)
				msg += (name + "\n");

			if (msg.empty())
				msg = "No Connected Clients";
			
			// Display block of text
			ImGui::InputTextMultiline("Clients", &msg[0], msg.size(), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 15), ImGuiInputTextFlags_ReadOnly);
		}

		ImGui::End();
	}

	
	/**
	 * Render loop is rather straight forward. 
	 * Render the text at the center of the screen.
	 */
	void WebSocketServerApp::render()
	{
		// Clear opengl context related resources that are not necessary any more
		mRenderService->destroyGLContextResources({ mRenderWindow.get() });

		// Activate current window for drawing
		mRenderWindow->makeActive();

		// Clear back-buffer
		mRenderService->clearRenderTarget(mRenderWindow->getBackbuffer());

		// Get render-able text component
		Renderable2DTextComponentInstance& text_comp = mTextEntity->getComponent<Renderable2DTextComponentInstance>();
		
		// Center
		text_comp.setLocation({ mRenderWindow->getWidthPixels() / 2, mRenderWindow->getHeightPixels() / 2 });
		
		// Draw
		text_comp.draw(mRenderWindow->getBackbuffer());

		// Draw gui to screen
		mGuiService->draw();

		// Swap screen buffers
		mRenderWindow->swap();
	}
	
	
	/**
	 * Occurs when the event handler receives a window message.
	 * You generally give it to the render service which in turn forwards it to the right internal window. 
	 * On the next update the render service automatically processes all window events. 
	 * If you want to listen to specific events associated with a window it's best to listen to a window's mWindowEvent signal
	 */
	void WebSocketServerApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}


	/**
	 * Called by the app loop. It's best to forward messages to the input service for further processing later on
	 * In this case we also check if we need to toggle full-screen or exit the running app
	 */
	void WebSocketServerApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// Escape the loop when esc is pressed
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			// If 'f' is pressed toggle fullscreen
			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				mRenderWindow->toggleFullscreen();
			}
		}

		mInputService->addEvent(std::move(inputEvent));
	}


	int WebSocketServerApp::shutdown()
	{
		return 0;
	}
}