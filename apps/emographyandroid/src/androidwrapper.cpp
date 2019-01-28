#include "emographyandroidapp.h"
#include "androidwrapper.h"

#include <nap/core.h>
#include <nap/logger.h>
#include <utility/errorstate.h>
#include <android/androidservicerunner.h>
#include <apiservice.h>
#include <nap/signalslot.h>
#include <apimessage.h>

namespace nap
{
    namespace android
    {
        /**
         * Describes the object that runs the emography android app.
         */
        using EmographyRunner = nap::AndroidServiceRunner<nap::EmographyAndroidApp, nap::AppEventHandler>;


        ///////////////////////////////////////////////////////
        // Callback
        ///////////////////////////////////////////////////////

        /**
         * Describes the signature of a NAP api callback function
         */
        using APICallbackFunction = std::function<void(JNIEnv *env, jobject context, const nap::APIEvent&)>;


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
        // Application Instance
        ///////////////////////////////////////////////////////

        /**
         * Thread safe emography nap application instance
         * For now implemented as a singleton, could be instantiated if required in the future
         * This is the case when multiple apps share this context?
         */
        class Instance final
        {
        public:
            /**
             * Creates the instance, only allowed if there is no instance created already.
             * @return if creation succeeded or not
             */
            bool init(JNIEnv *env, jobject contextObject)
            {
                if (mRunner != nullptr)
                {
                    nap::Logger::warn("instance of emography nap environment already exists!");
                    return false;
                }

                // Create core, replacing previous instance if allocated already.
                mCore   = std::make_unique<nap::Core>();
                mRunner = std::make_unique<EmographyRunner>(*mCore, env, contextObject);

                // Start
                nap::utility::ErrorState error;
                if (!mRunner->init(error))
                {
                    nap::Logger::fatal("error: %s", error.toString().c_str());
                    return false;
                }

                mAPIService = mCore->getService<nap::APIService>();
                if(mAPIService == nullptr)
                {
                    nap::Logger::fatal("error: %s", "unable to acquire handle to API service");
                    return false;
                }
                mAPIService->eventDispatched.connect(mDispatchSlot);

                return true;
            }

            /**
             * Shutdown the emography app environment
             * @return application exit code
             */
            int shutDown()
            {
                if(mRunner == nullptr)
                {
                    nap::Logger::warn("unable to shutdown nap env, not initialized");
                    return -1;
                }

                // Call shutdown reset ptr
                int v = mRunner->shutdown();
                mRunner.reset(nullptr);
                return v;
            }


            /**
             * Update the emography app environment
             */
            void update()
            {
                if(mRunner == nullptr)
                    nap::Logger::warn("unable to update nap env, not initialized");
                mRunner->update();
            }

            /**
             * @return the application runner, nullptr if not initialized
             */
            EmographyRunner* runner()
            {
                if(mRunner == nullptr)
                    nap::Logger::warn("unable to acquire app runner, not initialized");
                return mRunner.get();
            }

            /**
             * Installs a callback which is called when the system receives an api event
             * @param callback the callback to install
             */
            void installCallback(APICallbackFunction callback)
            {
                // Store callback
                mCallback = std::move(callback);
            }

            /**
             * Destructor
             */
            ~Instance()
            {
                mRunner.reset(nullptr);
                mCore.reset(nullptr);
            }

            /**
             * @return the global instance of this app.
             */
            static Instance& get()
            {
                static Instance sInstance;
                return sInstance;
            }


        private:
            // Hide constructor
            Instance()                                          {   }

            // Members
            nap::APIService* mAPIService                        = nullptr;
            std::unique_ptr<nap::Core> mCore                    = nullptr;
            std::unique_ptr<EmographyRunner> mRunner            = nullptr;
            APICallbackFunction mCallback                       = nullptr;

            /**
             * Called when the instance receives a message from the api service
             * @param event generated api event, forwarded to callback
             */
            void onAPIEventReceived(const nap::APIEvent& event)
            {
                if(mCallback == nullptr)
                    return;
                mCallback(mRunner->getApp().getEnv(), mRunner->getApp().getContext(), event);
            }

            // Slot that is connected on initialization
            nap::Slot<const nap::APIEvent&> mDispatchSlot       = {this, &Instance::onAPIEventReceived};
        };



        ///////////////////////////////////////////////////////
        // C calls
        ///////////////////////////////////////////////////////

        bool init(JNIEnv *env, jobject contextObject)
        {
            // Initialize the NAP env and app
            if(Instance::get().init(env, contextObject))
            {
                APICallbackFunction callbackFunc = &nap::android::apiEventReceived;
                Instance::get().installCallback(callbackFunc);
                return true;
            }
            return false;
        }


        void update(JNIEnv* env, jobject contextObject)
        {
            Instance::get().update();
        }


        void shutdown(JNIEnv* env, jobject contextObject)
        {
            Instance::get().shutDown();
        }


        void sendMessage(JNIEnv* env, jobject contextObject, jstring json)
        {
            EmographyRunner* runner = Instance::get().runner();
            if(runner == nullptr)
                return;

            //const char *s = env->GetStringUTFChars(jdata, NULL);
            //std::string data = s;
            //env->ReleaseStringUTFChars(jdata, s);
        }
    }
}
