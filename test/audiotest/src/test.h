#pragma once

#include <rtti/rtti.h>

class TestClass {
    RTTI_ENABLE()
public:
    void setX(float x)
    {
        mX = x;
    }
    
    float getX() const
    {
        return mX;
    }
    
private:
    float mX = 0;
};

