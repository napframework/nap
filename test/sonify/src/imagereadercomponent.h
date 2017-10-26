#pragma once

// Nap includes
#include <nap/component.h>
#include <nap/objectptr.h>
#include <rtti/rtti.h>
#include <image.h>

namespace nap {

    class ImageReaderComponentInstance;
    
    class NAPAPI ImageReaderComponent : public nap::Component
    {
        RTTI_ENABLE(nap::Component)
        DECLARE_COMPONENT(ImageReaderComponent, ImageReaderComponentInstance)
        
    public:
        ImageReaderComponent() : nap::Component() { }
        
        nap::ObjectPtr<nap::Image> mImage;
        
    private:
    };


    class NAPAPI ImageReaderComponentInstance : public nap::ComponentInstance
    {
        RTTI_ENABLE(nap::ComponentInstance)
    public:
        ImageReaderComponentInstance(nap::EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(nap::EntityCreationParameters& entityCreationParams, nap::utility::ErrorState& errorState) override;
        
        float getPixelIntensity(float x, float y);
        
    private:
        opengl::Bitmap* mBitmap = nullptr;
    };

}

