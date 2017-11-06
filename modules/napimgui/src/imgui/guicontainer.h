#pragma once

// Local includes
#include "imguiservice.h"

// External Includes
#include <rtti/rttiobject.h>
#include <renderwindow.h>

namespace nap
{
	/**
	 * This object associates a set of GUI objects with a window
	 * The Gui Service uses this link to set up the correct gui calls
	 * and makes sure all events are forwarded correctly. 
	 */
	class GuiContainer : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		/**
		 *	Constructor
		 */
		GuiContainer(nap::IMGuiService& service);
		
		/**
		 *	Destructor
		 */
		virtual ~GuiContainer();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		// property: link to the render window to associated this gui with
		ObjectPtr<RenderWindow> mWindow;

	private:
		IMGuiService& mService;
	};

	// Object creator used for constructing the gui container
	using GuiContainerObjectCreator = rtti::ObjectCreator<GuiContainer, IMGuiService>;
}
