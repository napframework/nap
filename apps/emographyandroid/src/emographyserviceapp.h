#ifndef PROJECT_EMOGRAPHYSERVICEAPP_H
#define PROJECT_EMOGRAPHYSERVICEAPP_H

#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <utility/errorstate.h>

#ifdef ANDROID
#include <jni.h>
#endif

// Some ugly workaround to allow us to run the app on Win64/macOS/Linux during dev
#ifdef ANDROID
	#include <android/androidserviceapp.h>
	typedef nap::AndroidServiceApp BaseClass;
#else // ANDROID
	#include <app.h>
	typedef nap::App BaseClass;
#endif // ANDROID

namespace nap
{
    using namespace rtti;

    class EmographyServiceApp : public BaseClass
    {
        RTTI_ENABLE(BaseClass)
    public:
        EmographyServiceApp(nap::Core& core);

        bool init(nap::utility::ErrorState& error) override;

        void update(double deltaTime) override;

        int shutdown() override;

        /**
         * Just a random call demonstrating.. basically not very much
         */
        void call(std::string data);

        std::string pullLogAndFlush();

    private:
        /**
         * Append a message to have transferred through to Java space. Super simple and lofi
         * demonstration. Currently using a pull from Java space to transfer this, partly to allow
         * the same mechanism from a native C++ app during dev.
         * @param message The message to log
         */
        void logToUI(std::string message);

        nap::SceneService* 			mSceneService = nullptr;
        nap::ResourceManager* 		mResourceManager = nullptr;

#ifdef ANDROID
        /**
         * Currently unused method to call back into the Android service from C++ space. Left as
         * an example, for eg. using from an event
         * @param message The message to log
         */
        void logToUIPush(std::string message);

        jmethodID 					mLogToUIMethodId = 0;
#endif

        int                         mCounter = 0;
        std::string                 mTimestamp = "";
        std::string                 mLogOutput = "";
    };

}

#endif //PROJECT_EMOGRAPHYSERVICEAPP_H
