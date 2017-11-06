#include "guicontainer.h"

// nap::gui_container run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GuiContainer)
	RTTI_CONSTRUCTOR(nap::IMGuiService&)
	RTTI_PROPERTY("Window", &GuiContainer::mWindow, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	GuiContainer::GuiContainer(nap::IMGuiService& service) : mService(service)
	{

	}

	GuiContainer::~GuiContainer() { }


	bool GuiContainer::init(utility::ErrorState& errorState)
	{
		return true;
	}
}