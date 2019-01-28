#ifndef PROJECT_EMOGRAPHYSERVICEAPP_H
#define PROJECT_EMOGRAPHYSERVICEAPP_H

// External Includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <utility/errorstate.h>
#include <jni.h>
#include <android/androidapp.h>
#include <apiservice.h>

namespace nap
{
    using namespace rtti;

    class EmographyAndroidApp : public AndroidApp
    {
        RTTI_ENABLE(AndroidApp)
    public:
        /**
         * Default constructor
         * @param core the nap core object
         */
        EmographyAndroidApp(nap::Core& core);

        /**
         * Initializes the application: fetches all required services and loads app json file.
         * @param error contains the error if the app couldn't be initialized.
         * @return if initialization failed or succeeded.
         */
        bool init(nap::utility::ErrorState& error) override;

        /**
         * Called from the external environment, updates internal resources and
         * @param deltaTime time in between calls
         */
        void update(double deltaTime) override;

        /**
         * Shuts down this application
         * @return exit return code, should be 0.
         */
        int shutdown() override;

    private:
        nap::SceneService* 			mSceneService = nullptr;            ///< Manages all scene related objects (entity, component etc.)
        nap::ResourceManager* 		mResourceManager = nullptr;         ///< Manages all the resources required by the APP.
        nap::APIService*            mAPIService = nullptr;              ///< Allows for sending and receiving nap API messages.
    };

}

#endif //PROJECT_EMOGRAPHYSERVICEAPP_H
