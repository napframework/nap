// Local Includes
#include "kalvertorenapp.h"
#include "selectledmeshcomponent.h"
#include "selectcolormethodcomponent.h"
#include "applytracercolorcomponent.h"
#include "applybbcolorcomponent.h"
#include "applycompositioncomponent.h"
#include "rendercompositioncomponent.h"

// External Includes
#include <mathutils.h>
#include <basetexture2d.h>
#include <texture2d.h>
#include <meshutils.h>
#include <imgui/imgui.h>
#include <imguiservice.h>
#include <utility/stringutils.h>
#include <scene.h>
#include <planemesh.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::KalvertorenApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	bool KalvertorenApp::init(utility::ErrorState& error)
	{
		// Create services
		renderService = getCore().getService<nap::RenderService>();
		inputService =  getCore().getService<nap::InputService>();
		sceneService =  getCore().getService<nap::SceneService>();

		// Get resource manager and load data
		resourceManager = getCore().getResourceManager();
		if (!resourceManager->loadFile("data/kalvertoren/kalvertoren.json", error))
			return false;    

		// Render window and texture target
		renderWindow = resourceManager->findObject<nap::RenderWindow>("Window0");
		
		// Callback when window event is received
		renderWindow->mWindowEvent.connect(std::bind(&KalvertorenApp::handleWindowEvent, this, std::placeholders::_1));

		// All of our entities
		ObjectPtr<Scene> scene = resourceManager->findObject<Scene>("Scene");

		// Entities
		compositionEntity = scene->findEntity("CompositionEntity");
		renderCompositionEntity = scene->findEntity("RenderCompositionEntity");
		displayEntity = scene->findEntity("DisplayEntity");
		sceneCameraEntity = scene->findEntity("SceneCameraEntity");
		compositionCameraEntity = scene->findEntity("CompositionCameraEntity");
		defaultInputRouter = scene->findEntity("DefaultInputRouterEntity");
		lightEntity = scene->findEntity("LightEntity");
		debugDisplayEntity = scene->findEntity("DebugDisplayEntity");

		// Materials
		vertexMaterial = resourceManager->findObject<nap::Material>("VertexColorMaterial");
		frameMaterial = resourceManager->findObject<nap::Material>("FrameMaterial");

		SelectLedMeshComponentInstance& selector = displayEntity->getComponent<SelectLedMeshComponentInstance>();
		mMeshSelection = selector.getIndex();

		// Set render states
		nap::RenderState& render_state = renderService->getRenderState();
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::PolygonMode::FILL;

		// Force paint method
		selectPaintMethod();

		// Position our debug windows
		positionDebugViews();

		return true;
	}

	void KalvertorenApp::update(double deltaTime)
	{
		nap::DefaultInputRouter& input_router = defaultInputRouter->getComponent<nap::DefaultInputRouterComponentInstance>().mInputRouter;

		// Update input for first window
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(sceneCameraEntity.get());
		inputService->processEvents(*renderWindow, input_router, entities);

		// Position the debug views
		positionDebugViews();

		// Update our gui
		updateGui();
	}


	void KalvertorenApp::render()
	{
		renderService->destroyGLContextResources(std::vector<nap::ObjectPtr<nap::RenderWindow>>({ renderWindow }));

		// Render offscreen surface(s)
		{
			renderService->getPrimaryWindow().makeCurrent();
			RenderCompositionComponentInstance& comp_render = renderCompositionEntity->getComponent<RenderCompositionComponentInstance>();
			comp_render.render();
		}


		// Render window 0
		{
			renderWindow->makeActive();

			// Clear backbuffer of window
			opengl::RenderTarget& backbuffer = renderWindow->getBackbuffer();
			renderService->clearRenderTarget(backbuffer);

			// Render debug views to screen
			renderDebugViews();

			// Get camera
			nap::CameraComponentInstance& sceneCamera = sceneCameraEntity->getComponent<nap::CameraControllerInstance>().getCameraComponent();

			// Render meshes
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			displayEntity->getComponentsOfType<nap::RenderableComponentInstance>(components_to_render);

			renderService->renderObjects(backbuffer, sceneCamera, components_to_render);

			// Render our gui
			getCore().getService<IMGuiService>()->render();

			renderWindow->swap();
		}
	}


	void KalvertorenApp::renderDebugViews()
	{
		for (auto& child : debugDisplayEntity->getChildren())
		{
			// Render our composition previs
			RenderableMeshComponentInstance& render_plane = child->getComponent<RenderableMeshComponentInstance>();
			OrthoCameraComponentInstance& ortho_cam = compositionCameraEntity->getComponent<OrthoCameraComponentInstance>();
			std::vector<nap::RenderableComponentInstance*> debug_objects = { &render_plane };

			// Get uniform to set
			UniformTexture2D& uniform = render_plane.getMaterialInstance().getOrCreateUniform<UniformTexture2D>("debugImage");

			// Set active texture
			if (utility::startsWith(child->mID, "CompositionDebugDisplayEntity"))
			{
				RenderCompositionComponentInstance& render_comp = renderCompositionEntity->getComponent<RenderCompositionComponentInstance>();
				uniform.setTexture(render_comp.getTexture());
			}
			else if (utility::startsWith(child->mID, "PaletteDebugDisplayEntity"))
			{
				ColorPaletteComponentInstance& palette_comp = compositionEntity->getComponent<ColorPaletteComponentInstance>();
				uniform.setTexture(palette_comp.getSelection());
			}

			renderService->renderObjects(renderWindow->getBackbuffer(), ortho_cam, debug_objects);
		}
	}


	void KalvertorenApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
		nap::rtti::TypeInfo e_type = windowEvent.get_type();
		if (e_type.is_derived_from(RTTI_OF(nap::WindowResizedEvent)) ||
			e_type.is_derived_from(RTTI_OF(nap::WindowShownEvent)))
		{
			positionDebugViews();
		}
	}


	// TODO: Possibly move these calls to seperate components
	void KalvertorenApp::positionDebugViews()
	{
		int y_loc = renderWindow->getHeight();
		for (auto& child : debugDisplayEntity->getChildren())
		{
			RenderableMeshComponentInstance& render_plane = child->getComponent<RenderableMeshComponentInstance>();
			assert(render_plane.getMesh().get_type().is_derived_from(RTTI_OF(nap::PlaneMesh)));
			nap::PlaneMesh& plane = static_cast<nap::PlaneMesh&>(render_plane.getMesh());

			nap::TransformComponentInstance& xform = render_plane.getEntityInstance()->getComponent<nap::TransformComponentInstance>();
			
			// We scale the palette based on the relative difference between the amount of available colors
			// in the index map
			int posy = y_loc - (plane.getRect().getHeight() / 2.0f);
			int posx = renderWindow->getWidth() - (plane.getRect().getWidth() / 2.0f);
			
			if (utility::startsWith(child->mID, "PaletteDebugDisplayEntity"))
			{
				ColorPaletteComponentInstance& palette_comp = compositionEntity->getComponent<ColorPaletteComponentInstance>();

				int color_palette_count = palette_comp.getSelection().getCount();
				int index_palette_count = palette_comp.getIndexMap().getCount();

				float div = (float)color_palette_count / (float)index_palette_count;
				xform.setScale({ div, 1.0f, 1.0f });

				float plane_width = plane.getRect().getWidth();
				posx -= ((plane_width/2.0) * (1.0-div));
			}

			// Set transform
			xform.setTranslate({ posx, posy, 0.0f });
			y_loc -= (plane.getRect().getHeight() + 10);
		}
	}


	void KalvertorenApp::shutdown()
	{

	}


	void KalvertorenApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		renderService->addEvent(std::move(windowEvent));
	}


	void KalvertorenApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
			{
				quit(0);
				return;
			}

			if (press_event->mKey == nap::EKeyCode::KEY_f)
			{
				static bool fullscreen = true;
				resourceManager->findObject<nap::RenderWindow>("Window0")->getWindow()->setFullScreen(fullscreen);
				fullscreen = !fullscreen;
				return;
			}
		}

		// Add event to input service for further processing
		inputService->addEvent(std::move(inputEvent));
	}


	void KalvertorenApp::updateGui()
	{
		// Get all the color selection components
		std::vector<SelectColorMethodComponentInstance*> color_methods;
		std::vector<ApplyTracerColorComponentInstance*>  tracer_painters;
		std::vector<ApplyCompositionComponentInstance*> composition_painters;

		for (auto& entity : compositionEntity->getChildren())
		{
			SelectColorMethodComponentInstance* color_method = &(entity->getComponent<SelectColorMethodComponentInstance>());
			color_methods.emplace_back(color_method);

			ApplyTracerColorComponentInstance* tracer_painter = &(entity->getComponent<ApplyTracerColorComponentInstance>());
			tracer_painters.emplace_back(tracer_painter);

			ApplyCompositionComponentInstance* comp_painter = &(entity->getComponent<ApplyCompositionComponentInstance>());
			composition_painters.emplace_back(comp_painter);
		}

		// Some extra comps
		SelectLedMeshComponentInstance& mesh_selector = displayEntity->getComponent<SelectLedMeshComponentInstance>();
		ColorPaletteComponentInstance& palette_selector =  compositionEntity->getComponent<ColorPaletteComponentInstance>();
		CompositionComponentInstance& composition_selector = compositionEntity->getComponent<CompositionComponentInstance>();

		ImGui::Begin("Display Settings");
		
		// Resets all the tracers
		if (ImGui::Button("Reset Walker"))
		{
			for (auto& tracer : tracer_painters)
			{
				tracer->reset();
			}
		}

		// Changes the mesh paint mode
		if (ImGui::Combo("Mode", &mPaintMode, "Channel Walker\0Bounding Box\0Composition\0\0"))
		{
			selectPaintMethod();
		}

		// Changes the display mesh
		if (ImGui::SliderInt("Mesh Selection", &mMeshSelection, 0, mesh_selector.getCount() - 1))
		{
			mesh_selector.select(mMeshSelection);
		}
		
		// Change tracer walk speed
		if (ImGui::SliderFloat("Channel Speed", &(mChannelSpeed), 0.0f, 20.0f))
		{
			for (auto& tracer : tracer_painters)
			{
				tracer->setSpeed(mChannelSpeed);
			}
		}
		
		// Allows the user to select a channel to display on the tracer
		if (ImGui::InputInt("Select Channel", &mSelectChannel, 1))
		{
			for (auto& tracer : tracer_painters)
			{
				tracer->selectChannel(mSelectChannel);
			}
		}

		// Show some additional info
		for(int i = 0; i<mesh_selector.getLedMeshes().size(); i++)
		{
			std::vector<std::string> parts;
			utility::splitString(mesh_selector.getLedMeshes()[i]->mTriangleMesh->mPath, '/', parts);
			
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), parts.back().c_str());
			ImGui::Text(utility::stringFormat("Channel: %d", i).c_str());
			const std::unordered_set<ArtNetController::Address>& addresses = mesh_selector.getLedMeshes()[i]->mTriangleMesh->getAddresses();
			
			std::string universes = "Addresses:";
			for (auto& address : addresses)
			{
				uint8 sub(0), uni(0);
				ArtNetController::convertAddress(address, sub, uni);
				universes += utility::stringFormat(" %d:%d", sub, uni);
			}
			
			ImGui::Text(universes.c_str());
		}
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),"%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		// Composition settings
		ImGui::Begin("Composition Settings");

		if (ImGui::Checkbox("Show Index Colors", &mShowIndexColors))
		{
			for (auto& comp_painter : composition_painters)
			{
				comp_painter->showIndexColors(mShowIndexColors);
			}
		}

		// Turn color cycling on / off
		if (ImGui::Checkbox("Cycle Color", &mCycleColors))
		{
			palette_selector.setCycle(mCycleColors);
		}

		// Changes the color palette
		if (ImGui::SliderInt("Composition", &mCompositionSelection, 0, composition_selector.getCount() - 1))
		{
			composition_selector.select(mCompositionSelection);
		}

		// Changes the color palette
		if (ImGui::SliderInt("Color Palette", &mPaletteSelection, 0, palette_selector.getCount() - 1))
		{
			palette_selector.select(mPaletteSelection);
		}

		// Changes the intensity
		if (ImGui::SliderFloat("Intensity", &mIntensity, 0.0f, 1.0f))
		{
			for (auto& comp_painter : composition_painters)
			{
				comp_painter->setIntensity(mIntensity);
			}
		}

		// Changes the duration
		if (ImGui::SliderFloat("Time Scale", &mDurationScale, 0.0f, 10.0f))
		{
			composition_selector.setDurationScale(mDurationScale);
		}

		// Changes the time at which a new color palette is selected
		if (ImGui::SliderFloat("Cycle Time", &mCycleTime, 0.0f, 10.0f))
		{
			palette_selector.setCycleSpeed(mCycleTime);
		}

		ImGui::End();

	}


	void KalvertorenApp::selectPaintMethod()
	{
		// Get all the color selection components
		std::vector<SelectColorMethodComponentInstance*> color_methods;

		for (auto& entity : compositionEntity->getChildren())
		{
			SelectColorMethodComponentInstance* color_method = &(entity->getComponent<SelectColorMethodComponentInstance>());
			color_methods.emplace_back(color_method);
		}

		for (auto& color_method : color_methods)
		{
			switch (mPaintMode)
			{
			case 0:
				color_method->select(RTTI_OF(nap::ApplyTracerColorComponentInstance));
				break;
			case 1:
				color_method->select(RTTI_OF(nap::ApplyBBColorComponentInstance));
				break;
			case 2:
				color_method->select(RTTI_OF(nap::ApplyCompositionComponentInstance));
				break;
			default:
				assert(false);
				break;
			}
		}
	}
}
