// Local Includes
#include "emographyandroidapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AndroidApp)
        RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
    EmographyAndroidApp::EmographyAndroidApp(nap::Core& core) : AndroidApp(core) { }

    bool EmographyAndroidApp::init(nap::utility::ErrorState& error)
    {
        mSceneService = getCore().getService<nap::SceneService>();
        mResourceManager = getCore().getResourceManager();
        mAPIService = getCore().getService<nap::APIService>();

        if (!mResourceManager->loadFile("emography.json", error))
            return false;

        return true;
    }


    void EmographyAndroidApp::update(double deltaTime)
    {
        std::unique_ptr<APIEvent> message(std::make_unique<APIEvent>("log"));
        message->addArgument<APIString>("info", "successfully loaded emography.json");
        mAPIService->dispatchEvent(std::move(message));

        std::unique_ptr<APIEvent> api_event(std::make_unique<APIEvent>("particle"));
        api_event->addArgument<APIFloat>("speed",1.0f);
        api_event->addArgument<APIInt>("drag", 2.0f);
        mAPIService->dispatchEvent(std::move(api_event));
    }


    int EmographyAndroidApp::shutdown()
    {
        return 0;
    }
}
