#include <nap.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

namespace nap
{
	class SFMLService : public Service
	{
		RTTI_ENABLE_DERIVED_FROM(Service)
	public:
		void addWindow(sf::RenderWindow* window)
		{
			mWindows.emplace_back(window);
			if (mWindows.size() == 1) runLoop();
        }

		void runLoop()
		{
			while (mWindows.size() > 0) {
				for (sf::RenderWindow* win : mWindows) {
					sf::Event event;
					while (win->pollEvent(event)) {
						if (event.type == sf::Event::Closed) win->close();
                        mDeadWindows.push_back(win);
					}
					win->clear(sf::Color::Blue);
					win->display();
				}
                // Clean up dead windows
                for (auto deadWin : mDeadWindows) {
                    mWindows.erase(std::remove(mWindows.begin(), mWindows.end(), deadWin), mWindows.end());
                }
                mDeadWindows.clear();
			}

		}

	private:
        std::vector<sf::RenderWindow*> mDeadWindows;
		std::vector<sf::RenderWindow*> mWindows;
	};
}

RTTI_DECLARE(nap::SFMLService)