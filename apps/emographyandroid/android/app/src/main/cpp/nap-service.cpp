#include <jni.h>

#include "napandroid.h"

extern "C" JNIEXPORT jlong JNICALL
Java_nl_naivi_emography_ForegroundService_napInit(JNIEnv* env, jobject contextObject)
{
    return nap::android::init(env, contextObject);
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_napUpdate(JNIEnv *env, jobject contextObject, jlong lp)
{
    nap::android::update(env, contextObject, lp);
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_napShutdown(JNIEnv *env, jobject contextObject, jlong lp)
{
    nap::android::shutdown(env, contextObject, lp);
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_napSendMessage(JNIEnv *env, jobject contextObject, jlong lp, jstring jdata)
{
    nap::android::sendMessage(env, contextObject, lp, jdata);
}

extern "C" JNIEXPORT jstring JNICALL
Java_nl_naivi_emography_ForegroundService_pullLogFromApp(JNIEnv *env, jobject contextObject, jlong lp)
{
    return nap::android::pullLogFromApp(env, contextObject, lp);
}