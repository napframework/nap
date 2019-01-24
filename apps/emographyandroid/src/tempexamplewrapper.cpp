#include "emographyandroidapp.h"
#include "tempexamplewrapper.h"

#include <nap/core.h>
#include <nap/logger.h>
#include <utility/errorstate.h>
#include <android/androidservicerunner.h>

namespace examplewrapper
{

    jlong initNap(JNIEnv* env, jobject contextObject)
    {
        nap::Logger::info("Creating nap::Core");

        // TODO Cleanup: deal with raw pointers

        // Create core
        nap::Core* core = new nap::Core();

        // Create service runner using default event handler
        nap::AndroidServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler>* service_runner =
                new nap::AndroidServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler>(*core, env, contextObject);

        // Start
        nap::utility::ErrorState error;
        if (!service_runner->init(error))
        {
            nap::Logger::fatal("error: %s", error.toString().c_str());
            return -1;
        }

        // Return pointer to service runner
        return (long) service_runner;        
    }


    void updateServiceRunner(JNIEnv* env, jobject contextObject, jlong lp)
    {
        nap::ServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler>& service_runner = getServiceRunner(lp);
        service_runner.update();
    }


    void shutdownNap(JNIEnv* env, jobject contextObject, jlong lp)
    {
        nap::ServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler>& service_runner = getServiceRunner(lp);
        nap::EmographyAndroidApp& app = service_runner.getApp();
        service_runner.shutdown();
        delete &app.getCore();
    }


    void callIntoNap(JNIEnv* env, jobject contextObject, jlong lp, jstring jdata)
    {
        nap::EmographyAndroidApp& app = getApp(lp);

        const char *s = env->GetStringUTFChars(jdata, NULL);
        std::string data = s;
        env->ReleaseStringUTFChars(jdata, s);

        app.call(data);
    }


    jstring pullLogFromApp(JNIEnv* env, jobject contextObject, jlong lp)
    {
        nap::EmographyAndroidApp& app = getApp(lp);
        return env->NewStringUTF(app.pullLogAndFlush().c_str());        
    }


     nap::ServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler>& getServiceRunner(jlong lp)
     {
         nap::ServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler> *service_runner =
                 (nap::ServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler> *) lp;
         return *service_runner;
     }


     nap::EmographyAndroidApp& getApp(jlong lp)
     {
         nap::ServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler>& service_runner = getServiceRunner(lp);
         return service_runner.getApp();
     }
}
