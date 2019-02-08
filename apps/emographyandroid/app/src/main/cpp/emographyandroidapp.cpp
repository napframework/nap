// Local Includes
#include "emographyandroidapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <emographystress.h>
#include <emographysummaryfunctions.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AndroidApp)
        RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
    using namespace emography;

    EmographyAndroidApp::EmographyAndroidApp(nap::Core& core) : AndroidApp(core) { }


    bool EmographyAndroidApp::init(nap::utility::ErrorState& error)
    {
        // Get necessary services.
        mSceneService = getCore().getService<nap::SceneService>();
        mResourceManager = getCore().getResourceManager();
        mAPIService = getCore().getService<nap::APIService>();
        mEmographyService = getCore().getService<nap::EmographyService>();

        // Set the database source directory: TODO populate from external environment directly
        mEmographyService->setDBSourceDir(getAppInternalFilesDir()+"/");

        // Load and spawn resources
        if (!mResourceManager->loadFile("emography.json", error))
            return false;

        // Find the data model.
        mDataModel = mResourceManager->findObject<emography::DataModel>("DataModel");
        assert(mDataModel != nullptr);

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
