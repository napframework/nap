#pragma once

// Mod nap render includes
#include <renderable2dtextcomponent.h>
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <websocketserverendpoint.h>
#include <app.h>
#include <color.h>
#include <renderservice.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop.
	 * 
	 * Demonstrates how to create a simple web-socket server setup using NAP. This demo uses the 
	 * nap::APIWebSocketServer to receive and respond to api messages that are received over a web-socket.
	 * When a 'ChangeText' nap api message is received the application changes the text on screen to
	 * the text that is extracted from the message. The received message should contain exactly 1 string argument.
	 *
	 * The request (in json) should look like this:
	 *
	 *   {
	 *		"Objects":
	 *		[
	 *			{
	 *				"Type": "nap::APIMessage",
	 *				"mID": "01",
	 *				"Name": "ChangeText",
	 *				"Arguments":
	 *				[
	 *					{
	 *						"Type": "nap::APIString",
	 *						"Name": "text",
	 *						"mID": "text_id",
	 *						"Value": "I'm so excited!"
	 *					}
	 *				]
	 *			}
	 *		]
	 *	}
	 *
	 * On success the server sends a 'TextChanged' reply message to the client. This message also contains
	 * (as the first argument) the updated text.
	 * 
	 * A simple html (javascript) web-socket client is bundled with this demo. 
	 * Use that client to send a web-socket message to this app and change the text in the render window. 
	 * The client is located inside the data folder of this app:
	 * 'data/websocket_html_client/websocket_client.html'. You can use any browser to run the demo client. On
	 * initialization the client will try to connect to the server using the 'uri' specified inside the .html file.
	 * Change the 'uri' to your computer's local ip address, for example: "ws://192.168.0.42:80".
	 * The default value is set to: "ws://localhost:80".
	 *
	 * For more information on web-sockets in nap refer to: nap::WebSocketServer and nap::WebSocketClient.
	 * For more information on using web-sockets in combination with the nap api refer to: nap::APIWebSocketServer 
	 * and nap::APIWebSocketClient. This example uses the nap::APIWebSocketServer in combination with a 
	 * nap::WebSocketServerEndPoint and nap::APIComponent. The end point manages all client connections, the
	 * server converts raw messages into nap api events, the api component receives those events on the main 
	 * thread and forwards them to potential handlers. 
	 *
	 * This application uses it's own custom event handler: 'nap::APIWebSocketHandler'. 
	 * This handler actually reacts and responds to the received query.
	 * The handler ships as part of this demo and is located inside the mod_websocketserver module.
	 */
	class WebSocketServerApp : public App
	{
		RTTI_ENABLE(App)
	public:
		// Constructor
		WebSocketServerApp(nap::Core& core) : App(core)	{ }

		/**
		 *	Initialize app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 *	Called when loop finishes
		 */
		int shutdown() override;

	private:
		// Nap Services
		RenderService* mRenderService = nullptr;						//< Render Service, handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input processes input events
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window		
		ObjectPtr<EntityInstance> mTextEntity = nullptr;				//< Pointer to the entity that renders text
		ObjectPtr<WebSocketServerEndPoint> mServerEndPoint = nullptr;	//< Pointer to the web-socket server endpoint, manages all client connections
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color

	};
}