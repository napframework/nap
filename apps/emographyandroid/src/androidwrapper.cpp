#include "emographyandroidapp.h"
#include "androidwrapper.h"
#include "androidinstance.h"

// External Includes
#include <apimessage.h>


namespace nap
{
    namespace android
    {
        ///////////////////////////////////////////////////////
        // Callback
        ///////////////////////////////////////////////////////

        /**
         * Called when the application receives an api event
         * @param env java native environment
         * @param context the android context in java
         * @param event the received api event
         */
        void apiEventReceived(JNIEnv *env, jobject context, const nap::APIEvent& event)
        {
            // Convert into api message
            APIMessage apimsg(event);

            // Extract json
            std::string json;
            nap::utility::ErrorState error;
            if(!apimsg.toJSON(json, error))
            {
                nap::Logger::error(error.toString());
                return;
            }

            // Calling for first time, get the method id
            jclass thisClass = env->GetObjectClass(context);
            jmethodID android_method = env->GetMethodID(thisClass, "logToUI", "(Ljava/lang/String;)V");

            jstring jstr = env->NewStringUTF(json.c_str());
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
                APICallbackFunction callbackFunc = &nap::android::apiEventReceived;
                Instance<EmographyAndroidApp>::get().installCallback(callbackFunc);
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


        void sendMessage(JNIEnv* env, jobject contextObject, jstring json)
        {
            /*
            auto* runner = Instance<EmographyAndroidApp>::get().runner();
            if(runner == nullptr)
                return;
            */
            //const char *s = env->GetStringUTFChars(jdata, NULL);
            //std::string data = s;
            //env->ReleaseStringUTFChars(jdata, s);
        }
    }
}
