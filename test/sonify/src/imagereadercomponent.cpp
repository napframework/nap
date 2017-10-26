#include "imagereadercomponent.h"

// Nap includes
#include <nap/entity.h>
#include <nap/configure.h>
#include <mathutils.h>

// Std includes

// RTTI
RTTI_BEGIN_CLASS(nap::ImageReaderComponent)
    RTTI_PROPERTY("Image", &nap::ImageReaderComponent::mImage, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ImageReaderComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("getPixelIntensity", &nap::ImageReaderComponentInstance::getPixelIntensity)
RTTI_END_CLASS

namespace nap {
    
    /**
     *    Utility function that returns a normalized float color value based on type T
     */
    template<typename T>
    void getPixelColor(const opengl::Bitmap& bitmap, const glm::ivec2& pixelCoordinates, glm::vec3& color, float divider)
    {
        T* pixel_color = bitmap.getPixel<T>(pixelCoordinates.x, pixelCoordinates.y);
        color.r = static_cast<float>(pixel_color[0]) / divider;
        color.g = static_cast<float>(pixel_color[1]) / divider;
        color.b = static_cast<float>(pixel_color[2]) / divider;
    }
    
    
    void getPixelColor(const opengl::Bitmap& bitmap, const glm::ivec2& pixelCoordinates, glm::vec3& color)
    {
        switch (bitmap.getDataType())
        {
            case opengl::BitmapDataType::BYTE:
                getPixelColor<nap::uint8>(bitmap, pixelCoordinates, color, math::max<nap::uint8>());
                break;
            case opengl::BitmapDataType::FLOAT:
                getPixelColor<float>(bitmap, pixelCoordinates, color, 1.0f);
                break;
            case opengl::BitmapDataType::USHORT:
                getPixelColor<nap::uint16>(bitmap, pixelCoordinates, color, math::max<nap::uint16>());
                break;
            default:
                assert(false);
                break;
        }
    }

    
    bool ImageReaderComponentInstance::init(nap::EntityCreationParameters& entityCreationParams, nap::utility::ErrorState& errorState)
    {
        mBitmap = &rtti_cast<ImageReaderComponent>(getComponent())->mImage->mBitmap;
        
        return true;
    }

    
    float ImageReaderComponentInstance::getPixelIntensity(float x, float y)
    {
        if (x > 1.0 || x < 0 || y > 1.0 || y < 0)
            return 0;
        
        glm::vec3 color;
        glm::ivec2 position;
        position.x = x * mBitmap->getWidth();
        position.y = y * mBitmap->getHeight();
        getPixelColor(*mBitmap, position, color);
        auto result = (color.x + color.y + color.z) / 3.f;
        return result;
    }
    

}
