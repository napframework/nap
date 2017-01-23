#include "operators.h"
#include "renderservice.h"

namespace nap
{
	ExecuteDrawOperator::ExecuteDrawOperator()
	{
		added.connect([&](nap::Object& args)
		{
			init();
		});
	}



	void ExecuteDrawOperator::init()
	{
		nap::Entity* root_entity = static_cast<nap::Entity*>(this->getRootObject());
		nap::RenderService* render_service = root_entity->getCore().getOrCreateService<RenderService>();

		std::vector<RenderWindowComponent*> windows;
		render_service->getObjects<RenderWindowComponent>(windows);
		assert(windows.size() > 0);
		mWindow = windows[0];

		mWindow->draw.signal.connect([&](const nap::SignalAttribute& attr)
		{
			drawOutputPlug.trigger();
		});
	}

}

RTTI_DEFINE(nap::ExecuteDrawOperator)