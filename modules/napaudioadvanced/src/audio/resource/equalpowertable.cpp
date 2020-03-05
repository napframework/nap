#include "equalpowertable.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::EqualPowerTable)
        RTTI_CONSTRUCTOR(nap::audio::NodeManager&)
        RTTI_PROPERTY("Size", &nap::audio::EqualPowerTable::mSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

    namespace audio
    {

        bool EqualPowerTable::init(utility::ErrorState& errorState)
        {
            if (mSize <= 0)
            {
                errorState.fail("Size must be greater than zero.");
                return false;
            }

            mTable = mNodeManager->makeSafe<EqualPowerTranslator<float>>(mSize);
            return true;
        }


    }

}
