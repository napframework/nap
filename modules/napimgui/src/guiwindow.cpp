#include "guiwindow.h"
#include <imgui/imgui_impl_sdl_gl3.h>

// nap::gui_container run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GuiWindow)
	RTTI_CONSTRUCTOR(nap::IMGuiService&)
	RTTI_PROPERTY("Window", &nap::GuiWindow::mWindow, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	GuiWindow::GuiWindow(nap::IMGuiService& service) : mService(service)
	{
	}


	GuiWindow::~GuiWindow()
	{
	}


	bool GuiWindow::init(utility::ErrorState& errorState)
	{
		mInitialized = true;
		return true;
	}

}