#ifndef PROJECT_TEMPEXAMPLEWRAPPER_H
#define PROJECT_TEMPEXAMPLEWRAPPER_H

#include "emographyserviceapp.h"

#include <jni.h>

#include <android/androidservicerunner.h>
#include <appeventhandler.h>

/**
 * A temporary little example of reducing the NAP specific code in the Android-space JNI wrapper
 */
namespace examplewrapper
{
    jlong initNap(JNIEnv* env, jobject contextObject);

    void updateServiceRunner(JNIEnv* env, jobject contextObject, jlong lp);

    void shutdownNap(JNIEnv* env, jobject contextObject, jlong lp);

    void callIntoNap(JNIEnv* env, jobject contextObject, jlong lp, jstring jdata);

    jstring pullLogFromApp(JNIEnv* env, jobject contextObject, jlong lp);

    nap::ServiceRunner<nap::EmographyServiceApp, nap::AppEventHandler>& getServiceRunner(jlong jp);

    nap::EmographyServiceApp& getApp(jlong jp);
}

#endif //PROJECT_TEMPEXAMPLEWRAPPER_H
