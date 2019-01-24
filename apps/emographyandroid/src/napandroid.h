#ifndef PROJECT_TEMPEXAMPLEWRAPPER_H
#define PROJECT_TEMPEXAMPLEWRAPPER_H

#include "emographyandroidapp.h"

#include <jni.h>

#include <android/androidservicerunner.h>
#include <appeventhandler.h>

/**
 * A temporary little example of reducing the NAP specific code in the Android-space JNI wrapper
 */
namespace  nap
{
    namespace android
    {
        jlong init(JNIEnv *env, jobject contextObject);

        void update(JNIEnv* env, jobject contextObject, jlong lp);

        void shutdown(JNIEnv* env, jobject contextObject, jlong lp);

        void sendMessage(JNIEnv* env, jobject contextObject, jlong lp, jstring jdata);

        jstring pullLogFromApp(JNIEnv* env, jobject contextObject, jlong lp);

        nap::ServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler>& getServiceRunner(jlong jp);

        nap::EmographyAndroidApp& getApp(jlong jp);
    }
}

#endif //PROJECT_TEMPEXAMPLEWRAPPER_H
