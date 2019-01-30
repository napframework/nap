#pragma once

// External Includes
#include <jni.h>

/**
 * A temporary little example of reducing the NAP specific code in the Android-space JNI wrapper
 */
namespace  nap
{
    namespace android
    {
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

        /**
         * Send a NAP api message as json to the running NAP application.
         * @param env
         * @param contextObject
         * @param json the NAP API message as json to send over
         */
        bool sendMessage(JNIEnv* env, jobject contextObject, jstring json);
    }
}