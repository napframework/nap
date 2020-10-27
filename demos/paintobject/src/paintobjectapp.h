/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <renderservice.h>
#include <imguiservice.h>
#include <parameterservice.h>
#include <app.h>
#include <spheremesh.h>
#include <font.h>
#include <imagefromfile.h>
#include <parameternumeric.h>
#include <parametercolor.h>
#include <parametergui.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that shows how to use multiple render texture components to draw on 3D objects
	 *
	 * Shows an object that can be painted using the left mouse button.
	 * By holding down the 'space bar' you can rotate the camera around the object
	 * 
	 * The application uses a rendertexturecomponent to generate a brush texture.
	 * Then the brush texture is used to render the brush stroke in another rendertexturecomponent
	 * The UV location of where to draw is determined by by tracing the position of the mouse on the mesh
	 * and then calculating the UV coordinates of that position on the mesh
	 *
	 * Mouse and key events are forwarded to the input service, the input service collects input events
	 * and processes all of them on update. Because NAP does not have a default space (objects can
	 * be rendered in multiple ways), you need to specify what input actually means to the application. 
	 * The input router does that for you. This demo uses the default one that forwards the events to every input component
	 * Refer to the cpp-update() call for more information on handling input
	 */
	class PaintObjectApp : public App
	{
		RTTI_ENABLE(App)
	public:
		PaintObjectApp(nap::Core& core) : App(core)	{ }

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
		void doTrace(const PointerEvent& event);

		void renderBrush();

		void renderPaint();

		void removeAllPaint();

		void switchMesh(int selection);

		// Nap Services
		RenderService*		mRenderService		= nullptr;				//< Render Service that handles render calls
		ResourceManager*	mResourceManager	= nullptr;				//< Manages all the loaded resources
		SceneService*		mSceneService		= nullptr;				//< Manages all the objects in the scene
		InputService*		mInputService		= nullptr;				//< Input service for processing input
		IMGuiService*		mGuiService			= nullptr;				//< Manages gui related update / draw calls
		ParameterService*	mParameterService	= nullptr;				//< Manages all parameters

		// Render Window
		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;	//< Pointer to the render window		

		// Entities
		ObjectPtr<EntityInstance>	mWorldEntity			= nullptr;	//< Pointer to the entity that holds the sphere
		ObjectPtr<EntityInstance>	mLightEntity			= nullptr;	//< Pointer to the entity that holds the sphere
		ObjectPtr<EntityInstance>	mPerspectiveCamEntity	= nullptr;	//< Pointer to the entity that holds the perspective camera
		ObjectPtr<EntityInstance>	mBrushEntity			= nullptr;

		// Render Textures
		ObjectPtr<RenderTexture2D>	mPaintTexture	= nullptr;	//< Pointer to the world texture
		ObjectPtr<RenderTexture2D>	mBrushTexture	= nullptr;	//< Pointer to the world texture
		
		// Parameters
		ObjectPtr<ParameterRGBColorFloat>	mBrushColorParam		= nullptr;
		ObjectPtr<ParameterFloat>			mBrushSizeParam			= nullptr;
		ObjectPtr<ParameterFloat>			mBrushSoftnessParam		= nullptr;
		ObjectPtr<ParameterFloat>			mBrushFalloffParam		= nullptr;
		ObjectPtr<ParameterFloat>			mLightIntensityParam	= nullptr;
		ObjectPtr<ParameterBool>			mEraserModeParam		= nullptr;
		ObjectPtr<ParameterInt>				mMeshSelectionParam		= nullptr;

		// Text Highlight Color
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color

		// GUI
		std::unique_ptr<ParameterGUI> mParameterGUI = nullptr;	//< Able to draw parameters to screen
		ObjectPtr<ParameterGroup> mParameterGroup = nullptr;		//< Contains all parameters

		// Draw state variables
		bool mMouseOnObject				= false;
		bool mMouseDown					= false;
		bool mDrawMode					= true;
		bool mClearPaint				= false;
		glm::vec2 mMousePosOnObject		= glm::vec2(0, 0);
		glm::vec4 mBrushColor			= glm::vec4(1, 0, 0, 1);

		// Selected mesh renderer
		std::string mSelectedMeshRendererID;
	};
}
