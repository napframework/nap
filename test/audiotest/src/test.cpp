#include "test.h"

RTTI_BEGIN_CLASS(TestClass)
    RTTI_FUNCTION("setX", &TestClass::setX)
    RTTI_FUNCTION("getX", &TestClass::getX)
RTTI_END_CLASS


namespace nap {
    
}
