#pragma once

// External Includes
#include <jni.h>
#include <nap/logger.h>
#include <apiservice.h>
#include <nap/signalslot.h>
#include <utility/errorstate.h>
#include <android/androidservicerunner.h>
#include <nap/core.h>
#include <mutex>

namespace  nap
{
    namespace android
    {
        /**
         * Various supported NAP android log levels
         */
        enum class ELogLevel : int
        {
            Info = 0,
            Warning = 1,
            Error = 2,
            Fatal = 4
        };

        /**
         * Describes the signature of a NAP log function
         */
        using APILogFunction = std::function<void(JNIEnv *env, jobject context, ELogLevel level, const std::string&)>;


        /**
         * Describes the signature of a NAP api callback function
         */
        using APICallbackFunction = std::function<void(JNIEnv *env, jobject context, const nap::APIEvent&)>;


        /**
         * Thread safe NAP android application instance, implemented as a singleton.
         * Where APP is the app that is instantiated and run using the AndroidServiceRunner.
         * Using this wrapper ensures all calls to a NAP app are thread safe and
         * all resources a de-allocated correctly on destruction.
         * Preferably init, update and shutdown are called from the same thread.
         * Messages can be received from any thread.
         */
        template<typename APP>
        class Instance final
        {
        public:
            using InstanceRunner = nap::AndroidServiceRunner<APP, nap::AppEventHandler>;

            /**
             * Creates the instance, only allowed if there is no instance created already.
             * @return if creation succeeded or not
             */
            bool init(JNIEnv *env, jobject contextObject)
            {
                std::lock_guard<std::mutex> lock(mInstanceMutex);
                if (mRunner != nullptr)
                {
                    log(ELogLevel::Error, "instance of emography nap environment already exists!");
                    return false;
                }

                // Create core, replacing previous instance if allocated already.
                mCore   = std::make_unique<nap::Core>();
                mRunner = std::make_unique<InstanceRunner>(*mCore, env, contextObject);

                // Start
                nap::utility::ErrorState error;
                if (!mRunner->init(error))
                {
                    log(ELogLevel::Error, error.toString());
                    return false;
                }

                mAPIService = mCore->getService<nap::APIService>();
                if(mAPIService == nullptr)
                {
                    log(ELogLevel::Error, "unable to acquire handle to API service");
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
                std::lock_guard<std::mutex> lock(mInstanceMutex);
                if(mRunner == nullptr)
                {
                    log(ELogLevel::Warning, "unable to shutdown nap env, not initialized");
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
                std::lock_guard<std::mutex> lock(mInstanceMutex);
                if(mRunner == nullptr)
                {
                    log(ELogLevel::Warning, "unable to update nap env, not initialized");
                    return;
                }
                mRunner->update();
            }


            /**
             * Sends an api message to the running application
             * json the api message as json string
             * error contains the error if sending fails
             */
             bool sendMessage(const char* json)
            {
                std::lock_guard<std::mutex> lock(mInstanceMutex);
                nap::utility::ErrorState error;
                if(!error.check(mRunner != nullptr, "unable to send message, not initialized"))
                {
                    log(ELogLevel::Warning, error.toString());
                    return false;
                }

                log(ELogLevel::Info, utility::stringFormat("Request: %s", json));
                if(!mAPIService->sendMessage(json, &error))
                {
                    log(ELogLevel::Error, error.toString());
                    return false;
                }
                return true;
            }


            /**
             * Installs a callback which is called when the system receives an api event
             * @param callback the callback to install
             */
            void installAPICallback(APICallbackFunction callback)
            {
                // Store callback
                mAPICallback = std::move(callback);
            }

            /**
             * Installs a callback which is called when the system receives a log message
             * @param callback the log message callback to install
             */
            void installLogCallback(APILogFunction callback)
            {
                 mLogCallback = std::move(callback);
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
            std::unique_ptr<InstanceRunner> mRunner             = nullptr;
            APICallbackFunction mAPICallback                    = nullptr;
            APILogFunction mLogCallback                         = nullptr;
            std::mutex mInstanceMutex;

            /**
             * Called when the instance receives a message from the api service
             * @param event generated api event, forwarded to callback
             */
            void onAPIEventReceived(const nap::APIEvent& event)
            {
                if(mAPICallback == nullptr)
                    return;
                mAPICallback(mRunner->getApp().getEnv(), mRunner->getApp().getContext(), event);
            }

            /**
             * Called when the system generates an api log message
             */
            void log(ELogLevel level, const std::string& msg)
            {
                 switch(level)
                 {
                     case ELogLevel::Info:
                         nap::Logger::info(msg);
                         break;
                     case ELogLevel::Warning:
                         nap::Logger::warn(msg);
                         break;
                     case ELogLevel::Error:
                         nap::Logger::error(msg);
                         break;
                     case ELogLevel::Fatal:
                         nap::Logger::fatal(msg);
                         break;
                 }

                 if(mLogCallback != nullptr)
                     mLogCallback(mRunner->getApp().getEnv(), mRunner->getApp().getContext(), level, msg);
            }


            // Slot that is connected on initialization
            nap::Slot<const nap::APIEvent&> mDispatchSlot       = {this, &Instance::onAPIEventReceived};
        };
    }
}
