#include "emographyandroidapp.h"
#include "androidwrapper.h"
#include "androidinstance.h"

// External Includes
#include <apimessage.h>


namespace nap
{
    namespace android {
        ///////////////////////////////////////////////////////
        // Callback
        ///////////////////////////////////////////////////////

        /**
         * Called when the application receives an api event
         * @param env java native environment
         * @param context the android context in java
         * @param event the received api event
         */
        void apiEventReceived(JNIEnv *env, jobject context, const nap::APIEvent &event) {
            // Convert into api message
            APIMessage apimsg(event);

            // Extract json, should ALWAYS succeed.
            std::string json;
            nap::utility::ErrorState error;
            if (!apimsg.toJSON(json, error))
            {
                assert(false);
                return;
            }

            // Calling for first time, get the method id
            jclass thisClass = env->GetObjectClass(context);
            jmethodID android_method = env->GetMethodID(thisClass, "logToUI", "(Ljava/lang/String;)V");


            jstring jstr = env->NewStringUTF(json.c_str());
            env->CallVoidMethod(context, android_method, jstr);
            env->DeleteLocalRef(jstr);
        }


        /**
         * Called when the application receives a log message
         * @param env java native environment
         * @param context the android context in java
         * @param level log level
         * @param msg the received message
         */
        void logMessageReceived(JNIEnv *env, jobject context, ELogLevel level, const std::string& msg)
        {
            // Calling for first time, get the method id
            jclass thisClass = env->GetObjectClass(context);
            jmethodID android_method = env->GetMethodID(thisClass, "logToUI", "(Ljava/lang/String;)V");

            jstring jstr = env->NewStringUTF(msg.c_str());
            env->CallVoidMethod(context, android_method, jstr);
            env->DeleteLocalRef(jstr);
        }



        ///////////////////////////////////////////////////////
        // C calls
        ///////////////////////////////////////////////////////

        bool init(JNIEnv *env, jobject contextObject)
        {
            // Initialize the NAP env and app
            if(Instance<EmographyAndroidApp>::get().init(env, contextObject))
            {
                // Install API callback
                APICallbackFunction callbackFunc = &nap::android::apiEventReceived;
                Instance<EmographyAndroidApp>::get().installAPICallback(callbackFunc);

                // Install log callback
                APILogFunction logFunc = &nap::android::logMessageReceived;
                Instance<EmographyAndroidApp>::get().installLogCallback(logFunc);
                return true;
            }
            return false;
        }


        void update(JNIEnv* env, jobject contextObject)
        {
            Instance<EmographyAndroidApp>::get().update();
        }


        void shutdown(JNIEnv* env, jobject contextObject)
        {
            Instance<EmographyAndroidApp>::get().shutDown();
        }


        bool sendMessage(JNIEnv* env, jobject contextObject, jstring json)
        {
            const char *json_msg = env->GetStringUTFChars(json, nullptr);
            return Instance<EmographyAndroidApp>::get().sendMessage(json_msg);
        }
    }
}
