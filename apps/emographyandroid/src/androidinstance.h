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
                    nap::Logger::warn("instance of emography nap environment already exists!");
                    return false;
                }

                // Create core, replacing previous instance if allocated already.
                mCore   = std::make_unique<nap::Core>();
                mRunner = std::make_unique<InstanceRunner>(*mCore, env, contextObject);

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
                std::lock_guard<std::mutex> lock(mInstanceMutex);
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
                std::lock_guard<std::mutex> lock(mInstanceMutex);
                if(mRunner == nullptr)
                    nap::Logger::warn("unable to update nap env, not initialized");
                mRunner->update();
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
            std::unique_ptr<InstanceRunner> mRunner             = nullptr;
            APICallbackFunction mCallback                       = nullptr;
            std::mutex mInstanceMutex;

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
    }
}
