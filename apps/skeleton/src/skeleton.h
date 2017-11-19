#pragma once

// Nap includes
#include <nap/resourcemanager.h>
#include <app.h>

namespace nap {
    /**
     * Main application that is called from within the main loop
     */
    class CoreApp : public App {
    public:
        CoreApp(nap::Core& core) : App(core) {}

        /**
         *	Provide CoreApp with the json data file that will be loaded, must be called before initializing.
         */
        void setFilename(const std::string& filename) { mFilename = filename; }

        /**
         *	Initialize all the services and app specific data structures
         */
        bool init(utility::ErrorState& error) override;

        /**
         *	Render is called after update, pushes all renderable objects to the GPU
         */
        void render() override;

        /**
         *
         *
         * @param deltaTime
         */
        void update(double deltaTime) override;

        /**
         *	Forwards the received window event to the render service
         */
        void windowMessageReceived(WindowEventPtr windowEvent) override;

        /**
         *  Forwards the received input event to the input service
         */
        void inputMessageReceived(InputEventPtr inputEvent) override;

        /**
         *	Called when loop finishes
         */
        void shutdown() override;


    private:
        // Nap Services
        ResourceManager* mResourceManager = nullptr;    //< Manages all the loaded data

        // The data file that will be used
        std::string mFilename;
    };
}
