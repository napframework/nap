// Local Includes
#include "imguiservice.h"
#include "imgui/imgui_impl_sdl_gl3.h"

// External Includes
#include <sceneservice.h>
#include <renderservice.h>
#include <inputservice.h>
#include <nap/core.h>

RTTI_DEFINE(nap::IMGuiService)

namespace nap
{

	void IMGuiService::render()
	{
		ImGui::Render();
	}

	bool IMGuiService::init(utility::ErrorState& error)
	{
		mRenderer = getCore().getService<nap::RenderService>();
		assert(mRenderer != nullptr);
		return true;
	}


	void IMGuiService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
		dependencies.emplace_back(RTTI_OF(RenderService));
		dependencies.emplace_back(RTTI_OF(InputService));
	}


	void IMGuiService::resourcesLoaded()
	{
		ImGui_ImplSdlGL3_Init(mRenderer->getPrimaryWindow().getNativeWindow());
	}


	void IMGuiService::update(double deltaTime)
	{
		mRenderer->getPrimaryWindow().makeCurrent();
		ImGui_ImplSdlGL3_NewFrame(mRenderer->getPrimaryWindow().getNativeWindow());
	}


	void IMGuiService::shutdown()
	{
		ImGui_ImplSdlGL3_Shutdown();
	}
}