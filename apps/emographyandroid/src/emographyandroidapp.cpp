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
    using namespace emography;

    EmographyAndroidApp::EmographyAndroidApp(nap::Core& core) : AndroidApp(core) { }

    bool EmographyAndroidApp::init(nap::utility::ErrorState& error)
    {
        mSceneService = getCore().getService<nap::SceneService>();
        mResourceManager = getCore().getResourceManager();
        mAPIService = getCore().getService<nap::APIService>();

        if (!mResourceManager->loadFile("emography.json", error))
            return false;

        std::string appInternalFilesDir = getAppInternalFilesDir();
        mDataModel = std::make_unique<DataModel>(mResourceManager->getFactory());
        if (!mDataModel->init(appInternalFilesDir + "/emography.db", DataModel::EKeepRawReadings::Disabled, error))
            return false;

        return true;
    }


    void EmographyAndroidApp::update(double deltaTime)
    {

    }


    int EmographyAndroidApp::shutdown()
    {
        return 0;
    }
}
