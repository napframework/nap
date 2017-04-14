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

		render_service->draw.signal.connect([&](const nap::SignalAttribute& attr)
		{
			drawOutputPlug.trigger();
		});
	}

}

RTTI_DEFINE(nap::ExecuteDrawOperator)