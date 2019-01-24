// Local Includes
#include "emographyandroidapp.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <emographysnapshot.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EmographyAndroidApp)
        RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
    EmographyAndroidApp::EmographyAndroidApp(nap::Core& core) : BaseClass(core) { }

    bool EmographyAndroidApp::init(nap::utility::ErrorState& error)
    {
        Logger::info("EmographyServiceApp::init");
        logToUI("EmographyServiceApp::init()");

        mSceneService = getCore().getService<nap::SceneService>();
        logToUI(" Got SceneService");

        mResourceManager = getCore().getResourceManager();
        logToUI(" Got ResourceManager");

        if (!mResourceManager->loadFile("dummy.json", error))
        {
            nap::Logger::error("Dummy load fail");
            return false;
        }

        logToUI(" Loaded dummy.json");

        std::string str = "data";
        emography::Snapshot<std::string> snapshot(str);
        logToUI(" Created an emography::Snapshot");

        return true;
    }


    void EmographyAndroidApp::update(double deltaTime)
    {
    }


    int EmographyAndroidApp::shutdown()
    {
        return 0;
    }


    void EmographyAndroidApp::logToUI(std::string message)
    {
        if (mLogOutput != "")
            mLogOutput += "\n";

        mLogOutput += message;
    }


    void EmographyAndroidApp::call(std::string data)
    {
        mTimestamp = data;
        mCounter++;
        Logger::info("EmographyServiceApp::call %d %s", mCounter, mTimestamp.c_str());
        logToUI(mTimestamp + " " + std::to_string(mCounter));
    }

    std::string EmographyAndroidApp::pullLogAndFlush()
    {
        // Simple demo, no threading considerations
        std::string s = mLogOutput;
        mLogOutput = "";
        return s;
    }

#ifdef ANDROID
    void EmographyAndroidApp::logToUIPush(std::string message)
    {
        JNIEnv* env = getEnv();
        if (env == nullptr)
            return;

        if (mLogToUIMethodId == 0)
        {
            // Calling for first time, get the method id
            jclass thisClass = env->GetObjectClass(mAndroidGlobalObject);
            mLogToUIMethodId = env->GetMethodID(thisClass, "logToUI", "(Ljava/lang/String;)V");
        }

        jstring jstr = env->NewStringUTF(message.c_str());
        env->CallVoidMethod(mAndroidGlobalObject, mLogToUIMethodId, jstr);
        env->DeleteLocalRef(jstr);
    }
#endif // ANDROID
}
