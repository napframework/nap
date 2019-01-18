#include "emographyserviceapp.h"

#include <nap/core.h>
#include <nap/logger.h>

#include <emographysnapshot.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EmographyServiceApp)
        RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
    EmographyServiceApp::EmographyServiceApp(nap::Core& core) : BaseClass(core) { }

    bool EmographyServiceApp::init(nap::utility::ErrorState& error)
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


    void EmographyServiceApp::update(double deltaTime)
    {
    }


    int EmographyServiceApp::shutdown()
    {
        return 0;
    }


    void EmographyServiceApp::logToUI(std::string message)
    {
        if (mLogOutput != "")
            mLogOutput += "\n";

        mLogOutput += message;
    }


    void EmographyServiceApp::call(std::string data)
    {
        mTimestamp = data;
        mCounter++;
        Logger::info("EmographyServiceApp::call %d %s", mCounter, mTimestamp.c_str());
        logToUI(mTimestamp + " " + std::to_string(mCounter));
    }

    std::string EmographyServiceApp::pullLogAndFlush()
    {
        // Simple demo, no threading considerations
        std::string s = mLogOutput;
        mLogOutput = "";
        return s;
    }

#ifdef ANDROID
    void EmographyServiceApp::logToUIPush(std::string message)
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
