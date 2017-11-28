#include <utility/fileutils.h>
#include <nap/logger.h>
#include "skeleton.h"

namespace nap {
    /**
     * Initialize all the resources and instances used for drawing
     * slowly migrating all functionality to nap
     */
    bool CoreApp::init(utility::ErrorState& error)
    {
        mResourceManager = getCore().getResourceManager();
        auto abspath = utility::getAbsolutePath(mFilename);
        nap::Logger::info("Loading: %s", abspath.c_str());
        if (!mResourceManager->loadFile(mFilename, error))
            return false;

        return true;
    }


    // Called when the window is going to render
    void CoreApp::render()
    {
    }


    void CoreApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
    }


    void CoreApp::inputMessageReceived(InputEventPtr inputEvent)
    {
    }


    void CoreApp::shutdown()
    {

    }

    void CoreApp::update(double deltaTime)
    {
        mResourceManager->checkForFileChanges();
    }
}
