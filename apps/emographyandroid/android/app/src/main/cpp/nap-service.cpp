#include <jni.h>

#include "tempexamplewrapper.h"

extern "C" JNIEXPORT jlong JNICALL
Java_nl_naivi_emography_ForegroundService_initNap(JNIEnv* env, jobject contextObject)
{
    return examplewrapper::initNap(env, contextObject);
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_updateServiceRunner(JNIEnv *env, jobject contextObject, jlong lp)
{
    examplewrapper::updateServiceRunner(env, contextObject, lp);
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_shutdownNap(JNIEnv *env, jobject contextObject, jlong lp)
{
    examplewrapper::shutdownNap(env, contextObject, lp);
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_callIntoNap(JNIEnv *env, jobject contextObject, jlong lp, jstring jdata)
{
    examplewrapper::callIntoNap(env, contextObject, lp, jdata);
}

extern "C" JNIEXPORT jstring JNICALL
Java_nl_naivi_emography_ForegroundService_pullLogFromApp(JNIEnv *env, jobject contextObject, jlong lp)
{
    return examplewrapper::pullLogFromApp(env, contextObject, lp);
}