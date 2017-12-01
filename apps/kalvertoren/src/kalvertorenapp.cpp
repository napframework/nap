// Local Includes
#include "kalvertorenapp.h"
#include "selectledmeshcomponent.h"
#include "selectcolormethodcomponent.h"
#include "applytracercolorcomponent.h"
#include "applybbcolorcomponent.h"
#include "applycompositioncomponent.h"

// External Includes
#include <mathutils.h>
#include <basetexture2d.h>
#include <texture2d.h>
#include <meshutils.h>
#include <imgui/imgui.h>
#include <imguiservice.h>
#include <utility/stringutils.h>
#include <scene.h>

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
		videoService =  getCore().getService<nap::VideoService>();

		// Get resource manager and load data
		resourceManager = getCore().getResourceManager();
		if (!resourceManager->loadFile("data/kalvertoren/kalvertoren.json", error))
			return false;

		// Render window and texture target
		renderWindow = resourceManager->findObject<nap::RenderWindow>("Window0");
		
		// All of our entities
		ObjectPtr<Scene> scene = resourceManager->findObject<Scene>("Scene");

		compositionEntity = scene->findEntity("CompositionEntity");
		displayEntity = scene->findEntity("DisplayEntity");
		sceneCameraEntity = scene->findEntity("SceneCameraEntity");
		compositionCameraEntity = scene->findEntity("CompositionEntity");
		defaultInputRouter = scene->findEntity("DefaultInputRouterEntity");
		lightEntity = scene->findEntity("LightEntity");

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

		selectPaintMethod();

		return true;
	}

	void KalvertorenApp::update(double deltaTime)
	{
		nap::DefaultInputRouter& input_router = defaultInputRouter->getComponent<nap::DefaultInputRouterComponentInstance>().mInputRouter;

		// Update input for first window
		std::vector<nap::EntityInstance*> entities;
		entities.push_back(sceneCameraEntity.get());
		inputService->processEvents(*renderWindow, input_router, entities);

		// Update our gui
		updateGui();
	}


	void KalvertorenApp::render()
	{
		renderService->destroyGLContextResources(std::vector<nap::ObjectPtr<nap::RenderWindow>>({ renderWindow }));

		// Render offscreen surface(s)
		{
		}


		// Render window 0
		{
			renderWindow->makeActive();

			nap::CameraComponentInstance& sceneCamera = sceneCameraEntity->getComponent<nap::CameraControllerInstance>().getCameraComponent();

			// Render output texture to plane
			std::vector<nap::RenderableComponentInstance*> components_to_render;
			displayEntity->getComponentsOfType<nap::RenderableComponentInstance>(components_to_render);

			opengl::RenderTarget& backbuffer = *(opengl::RenderTarget*)(renderWindow->getWindow()->getBackbuffer());
			renderService->clearRenderTarget(backbuffer);
			renderService->renderObjects(backbuffer, sceneCamera, components_to_render);

			// Render our gui
			getCore().getService<IMGuiService>()->render();

			renderWindow->swap();
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

		// Gui
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
