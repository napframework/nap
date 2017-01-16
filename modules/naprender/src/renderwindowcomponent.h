#pragma once

#include <nap.h>

#include "renderservice.h"

namespace nap
{
	class RenderWindowComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)

	public:
		RenderWindowComponent()
		{

			added.connect([=](Object& parent) { createWindow(); });

			removed.connect([=](Object& parent) { destroyWindow(); });

			visible.valueChanged.connect([=](AttributeBase& attrib) {
				if (rtti_cast<Attribute<bool>*>(&attrib)->getValue())
					showWindow();
				else
					hideWindow();
			});

			visible.setFlag(Editable, true);
			int flags = visible.getFlags();
			Logger::debug("%d", flags);
		}

		void createWindow()
		{
			auto service = getRoot()->getCore().getOrCreateService<RenderService>();
			mWindow = service->getWindow();
		}

		void destroyWindow()
		{
			auto service = getRoot()->getCore().getOrCreateService<RenderService>();
			service->destroyWindow(mWindow);
		}

		void hideWindow() { destroyWindow(); }

		void showWindow() { createWindow(); }

		Attribute<bool> visible = {this, "visible", true};

	private:
		SDL_Window* mWindow = nullptr;
	};
}

RTTI_DECLARE(nap::RenderWindowComponent)