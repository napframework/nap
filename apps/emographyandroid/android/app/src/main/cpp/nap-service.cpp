#include <jni.h>

#include "androidwrapper.h"

extern "C" JNIEXPORT bool JNICALL
Java_nl_naivi_emography_ForegroundService_napInit(JNIEnv* env, jobject contextObject)
{
    return nap::android::init(env, contextObject);
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_napUpdate(JNIEnv *env, jobject contextObject)
{
    nap::android::update(env, contextObject);
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_napShutdown(JNIEnv *env, jobject contextObject)
{
    nap::android::shutdown(env, contextObject);
}

extern "C" JNIEXPORT void JNICALL
Java_nl_naivi_emography_ForegroundService_napSendMessage(JNIEnv *env, jobject contextObject, jstring jdata)
{
    nap::android::sendMessage(env, contextObject, jdata);
}

extern "C" JNIEXPORT jstring JNICALL
Java_nl_naivi_emography_ForegroundService_pullLogFromApp(JNIEnv *env, jobject contextObject)
{
    return nap::android::pullLogFromApp(env, contextObject);
}