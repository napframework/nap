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
         * Describes the signature of a NAP log function
         */
        using APILogFunction = std::function<void(JNIEnv *env, jobject context, int level, const std::string&)>;


        /**
         * Describes the signature of a NAP api callback function
         */
        using APICallbackFunction = std::function<void(JNIEnv *env, jobject context, const nap::APIEvent&)>;


        /**
         * Thread safe NAP android application instance, implemented as a singleton.
         * Where APP is the app that is instantiated and run using the AndroidServiceRunner.
         * Using this wrapper ensures all calls to a NAP app are thread safe and
         * all resources a de-allocated correctly on destruction.
         * Init, Update and Shutdown must be called from the same thread.
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
                if (mInitialized)
                {
                    nap::Logger::warn("instance of emography nap environment already exists!");
                    return false;
                }

                // Create core and application
                create(env, contextObject);

                // Initialize app and start running
                nap::utility::ErrorState error;
                nap::Logger::info("initializing NAP application");
                if (!mRunner->init(error))
                {
                    nap::Logger::error(error.toString());
                    destroy();
                    return false;
                }

                // Fetch API service
                mAPIService = mCore->getService<nap::APIService>();
                assert(mAPIService != nullptr);
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
                if(!mInitialized)
                {
                    nap::Logger::error("unable to shutdown nap env, not initialized");
                    return -1;
                }
                return destroy();
            }


            /**
             * Update the emography app environment
             * Can only be called when initialization succeeded!
             */
            void update()
            {
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
                if(!error.check(mInitialized, "unable to send message, not initialized"))
                {
                    nap::Logger::warn(error.toString());
                    return false;
                }

                // Forward message
                if(!mAPIService->sendMessage(json, &error))
                {
                    nap::Logger::error(error.toString());
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
                if(mInitialized)
                    destroy();
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
            Instance() = default;

            /**
             * Shuts down the app environment
             * @return application exit code
             */
            int destroy()
            {
                Logger::instance().log.disconnect(mLogSlot);
                nap::Logger::info("shutting down NAP application");
                int v = mRunner->shutdown();
                mRunner.reset(nullptr);
                mCore.reset(nullptr);
                mInitialized = false;
                nap::Logger::info("destroyed NAP application");
                return v;
            }

            /**
             * Creates the app environment
             * @param env the java native environment
             * @param contextObject java context
             */
            void create(JNIEnv *env, jobject contextObject)
            {
                // Create core, replacing previous instance if allocated already.
                mCore   = std::make_unique<nap::Core>();
                mRunner = std::make_unique<InstanceRunner>(*mCore, env, contextObject);
                Logger::instance().log.connect(mLogSlot);
                mInitialized = true;
                nap::Logger::info("created NAP application");
            }

            // Members
            nap::APIService* mAPIService                        = nullptr;
            std::unique_ptr<nap::Core> mCore                    = nullptr;
            std::unique_ptr<InstanceRunner> mRunner             = nullptr;
            APICallbackFunction mAPICallback                    = nullptr;
            APILogFunction mLogCallback                         = nullptr;
            std::mutex mInstanceMutex;
            bool mInitialized = false;

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
             * Called when the instance receives a log message from the NAP logger
             * @param message generated log message, forwarded to callback
             */
            void onLogMessageReceived(nap::LogMessage message)
            {
                // Send if there's a callback
                if(mLogCallback == nullptr)
                    return;

                // Forward
                mLogCallback(mRunner->getApp().getEnv(), mRunner->getApp().getContext(),
                        message.level().level(),
                        message.text());
            }


            // Slot that is connected on initialization
            nap::Slot<const nap::APIEvent&> mDispatchSlot = {this, &Instance::onAPIEventReceived};
            nap::Slot<nap::LogMessage> mLogSlot = {this, &Instance::onLogMessageReceived};
        };
    }
}
