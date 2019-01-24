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
        /**
         * Describes the object that runs the emography android app.
         */
        using EmographyRunner = nap::AndroidServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler>;

        /**
         * Initializes NAP and the wrapped Emography application
         * @param env JNI the Java Native Interface
         * @param contextObject object that calls into this function
         * @return if initialization succeeded or failed
         */
        bool init(JNIEnv *env, jobject contextObject);

        /**
         * Updates the running app
         * @param env JNI the Java Native Interface
         * @param contextObject object that calls into this function
         */
        void update(JNIEnv* env, jobject contextObject);

        /**
         * Shuts down the running app and nap contect
         * @param env JNI the Java Native Interface
         * @param contextObject object that calls into this function
         */
        void shutdown(JNIEnv* env, jobject contextObject);




        void sendMessage(JNIEnv* env, jobject contextObject, jstring jdata);

        jstring pullLogFromApp(JNIEnv* env, jobject contextObject);
    }
}

#endif //PROJECT_TEMPEXAMPLEWRAPPER_H
