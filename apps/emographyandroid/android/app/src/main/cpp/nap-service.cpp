#include <jni.h>

#include <nap/core.h>
#include <nap/logger.h>
#include <utility/errorstate.h>
#include <android/androidservicerunner.h>
#include <appeventhandler.h>

#include "emographyserviceapp.h"

extern "C" JNIEXPORT jlong JNICALL
Java_nl_naivi_emography_ForegroundService_initNap(JNIEnv* env, jobject thiz)
{
    nap::Logger::info("Creating nap::Core");

    // TODO Cleanup: deal with raw pointers

    // Create core
    nap::Core* core = new nap::Core();

    // Create service runner using default event handler
    nap::AndroidServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler>* service_runner =
            new nap::AndroidServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler>(*core, env, thiz);

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

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_updateServiceRunner(JNIEnv *env, jobject thiz, jlong lp)
{
    nap::ServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler> *service_runner =
            (nap::ServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler> *) lp;
    service_runner->update();
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_shutdownNap(JNIEnv *env, jobject thiz, jlong lp)
{
    nap::ServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler> *service_runner =
            (nap::ServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler> *) lp;
    nap::EmographyServiceApp& app = service_runner->getApp();
    service_runner->shutdown();
    delete &app.getCore();
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_callIntoNap(JNIEnv *env, jobject thiz, jlong lp, jstring jdata)
{
    nap::ServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler> *service_runner =
            (nap::ServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler> *) lp;
    nap::EmographyServiceApp& app = service_runner->getApp();

    const char *s = env->GetStringUTFChars(jdata, NULL);
    std::string data = s;
    env->ReleaseStringUTFChars(jdata, s);

    app.call(data);
}

extern "C" JNIEXPORT jstring JNICALL
Java_nl_naivi_emography_ForegroundService_pullLogFromApp(JNIEnv *env, jobject thiz, jlong lp)
{
    nap::ServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler> *service_runner =
            (nap::ServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler> *) lp;
    nap::EmographyServiceApp &app = service_runner->getApp();
    return env->NewStringUTF(app.pullLogAndFlush().c_str());
}