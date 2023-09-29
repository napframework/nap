/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayercurveoutput.h"
#include "sequenceservice.h"
#include "sequenceplayercurveadapter.h"

#include <nap/logger.h>
#include <parametersimple.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerCurveOutput)
        RTTI_PROPERTY("Parameter", &nap::SequencePlayerCurveOutput::mParameter, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("Use Main Thread", &nap::SequencePlayerCurveOutput::mUseMainThread, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    static std::vector<rtti::TypeInfo> allowed_parameter_types =
    {
        RTTI_OF(ParameterFloat),
        RTTI_OF(ParameterBool),
        RTTI_OF(ParameterDouble),
        RTTI_OF(ParameterLong),
        RTTI_OF(ParameterInt),
        RTTI_OF(ParameterVec2),
        RTTI_OF(ParameterVec3),
        RTTI_OF(ParameterVec4)
    };

    SequencePlayerCurveOutput::SequencePlayerCurveOutput(SequenceService& service)
        : SequencePlayerOutput(service)
    {
    }


    void SequencePlayerCurveOutput::update(double deltaTime)
    {
        for(auto *curve_adapter: mAdapters)
        {
            curve_adapter->setValue();
        }
    }


    void SequencePlayerCurveOutput::registerAdapter(SequencePlayerCurveAdapterBase* curveAdapter)
    {
        auto found_it = std::find_if(mAdapters.begin(), mAdapters.end(), [&](const auto &it)
        {
            return it == curveAdapter;
        });
        assert(found_it == mAdapters.end()); // duplicate entry

        if(found_it == mAdapters.end())
        {
            mAdapters.emplace_back(curveAdapter);
        }
    }


    void SequencePlayerCurveOutput::removeAdapter(SequencePlayerCurveAdapterBase* curveAdapter)
    {
        auto found_it = std::find_if(mAdapters.begin(), mAdapters.end(), [&](const auto &it)
        {
            return it == curveAdapter;
        });

        if(found_it != mAdapters.end())
        {
            mAdapters.erase(found_it);
        }
    }


    bool SequencePlayerCurveOutput::init(utility::ErrorState &errorState)
    {
        if(!errorState.check(std::find(allowed_parameter_types.begin(),
                                       allowed_parameter_types.end(),
                                       mParameter->get_type())!=allowed_parameter_types.end(),
                             utility::stringFormat("Parameter %s not allowed", std::string(mParameter->get_type().get_name()).c_str())))
        {
            return false;
        }

        return SequencePlayerOutput::init(errorState);
    }
}