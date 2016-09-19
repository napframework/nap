#include "operators.h"
#include "sfmlservice.h"
#include <SFML/Graphics.hpp>
#include <nap.h>


namespace nap
{
	class SFMLWindowComponent : public nap::Component
	{
		RTTI_ENABLE_DERIVED_FROM(nap::Component)
	public:
		SFMLWindowComponent() : nap::Component()
		{
			added.connect([&](const nap::Object& ob) { onAdded(); });
		}

		void runLoop() {}

		sf::Window& getWindow() { return *mWindow.get(); }

		nap::Attribute<bool> runMainLoop = {this, "runMainLoop", true};

	private:
		void onAdded()
		{
			mWindow = std::make_unique<sf::RenderWindow>(sf::VideoMode(800, 600), "Window");

//			nap::SFMLService& service = getParent()->getCore().getOrCreateService<nap::SFMLService>();
//			service.addWindow(mWindow.get());
		}

		std::unique_ptr<sf::RenderWindow> mWindow;
		nap::SFMLService* mService;
	};
}

RTTI_DECLARE(nap::SFMLWindowComponent)

NAP_MODULE_BEGIN(SFML)
{
    NAP_REGISTER_SERVICE(nap::SFMLService)

	NAP_REGISTER_DATATYPE(sf::RenderWindow)

	NAP_REGISTER_OPERATOR(CreateRenderWindowOperator)

	NAP_REGISTER_COMPONENT(nap::SFMLWindowComponent)
}
NAP_MODULE_END(SFML)
