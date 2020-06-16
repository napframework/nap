#pragma once

// Local includes
#include "heightmesh.h"

// Nap includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <imguiservice.h>
#include <renderservice.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <app.h>
#include <smoothdamp.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop
	 *
	 * Shows a grid that is displaced using a height map.
	 * This application uses 2 new resources: "HeightMesh" and "HeightNormals", both are meshes
	 * These meshes are defined in mod_heightmap, shipped together with this demo.
	 * The app module is compiled in to a dynamic link library, this ensures the objects can be created and adjusted in the editor
	 * 
	 * The height mesh is a plane that links to an image (height map). The plane is displaced along it's normal based on the information
	 * in the image. The normals of the plane are visualized using the height normals mesh. This mesh links to the plane and creates a
	 * normal for every vertex associated with that plane. It also creates some extra vertex attributes to display interpolated normal values
	 * 
	 * The displacement can be adjusted using a blend value. When set to 0 the plane's vertices are not displaced, when set to 1
	 * the vertices are fully displaced. The maximum displacement value is a HeightMesh property.
	 * Shaders are used to calculate the new vertex position based on the vertex attributes associated with both meshes. 
	 * The height mesh stores it's original position alongside the current (max) displacement value. The shader uses that information
	 * to find the right displaced position based on a blend value. Normals are also blended based on the current blend value.
	 * Having access to both start and end vertex data allows us to efficiently compute in-between values, all computations occur on the GPU
	 * Having to re-calculate the normals on the CPU is heavy on a 1024x1024 triangle grid, this is now handled for us by the GPU
	 *
	 * The GUI allows you to toggle normal blending on/off and change the height blend value. When normal blending is turned off
	 * the normals keep pointing upwards, causing the halo effect to be exactly the same with or without displacement.
	 * When blending is turned on the normals are interpolated between the min max displacement values, 
	 * with as a result a more accurate normal representation.
	 *
	 * This demo is relatively heavy as it draws over 3 million primitives to screen, where 1 million primitives are transparent.
	 * It runs fine on most dedicated gpu's (150+ fps) but doesn't work as well on integrated graphics cards (intel etc.).
	 * 
	 */
	class HeightmapApp : public App
	{
		RTTI_ENABLE(App)
	public:
		HeightmapApp(nap::Core& core) : App(core)	{ }

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
		RenderService*			mRenderService = nullptr;			//< Render Service that handles render calls
		ResourceManager*		mResourceManager = nullptr;			//< Manages all the loaded resources
		SceneService*			mSceneService = nullptr;			//< Manages all the objects in the scene
		InputService*			mInputService = nullptr;			//< Input service for processing input
		IMGuiService*			mGuiService = nullptr;				//< Gui service

		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;		//< Pointer to the render window
		ObjectPtr<EntityInstance>	mCameraEntity = nullptr;		//< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance>	mWorldEntity = nullptr;			//< Pointer to the entity that holds the sphere
		ObjectPtr<ImageFromFile>	mHeightmap = nullptr;			//< Texture used as height map

		// Gui variables
		float	mBlendValue = 1.0f;									//< Height blend value
		int		mSelection = 2;										//< What we want to display to screen
		float	mNormalOpacity = 0.3;								//< Opacity of the normals
		float	mNormalLength = 0.5;								//< Length of the normals on screen
		bool	mBlendNormals = true;								//< If the normal visualization is blended
		RGBColorFloat mNormalColor = { 1.000f, 1.000f, 1.000f };	//< Color of the normal
		RGBColorFloat mValleyColor = { 0.176f, 0.180f, 0.258f };	//< Color of the valley
		RGBColorFloat mPeakColor =	 { 0.784f, 0.411f, 0.411f };	//< Color of the peak
		RGBColorFloat mHaloColor =	 { 0.545f, 0.549f, 0.627f };	//< Color of the halo
		RGBColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69};		//< GUI text highlight color

		// Value Smoother
		math::FloatSmoothOperator mBlendSmoother = { 1.0f, 0.5f };	//< smooths blend value over time to target value

		// Updates gui components
		void updateGui();

		// Pushes an rgb color to a shader uniform associated with a material
		void pushColor(RGBColorFloat& color, MaterialInstance& material, const std::string& uboName, const std::string& uniformName);
	};
}
