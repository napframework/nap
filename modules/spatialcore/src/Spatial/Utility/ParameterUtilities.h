#pragma once

#include <vector>
#include <utility/dllexport.h>

namespace nap
{
    class ParameterGroup;
    
    namespace spatial
    {
        void NAPAPI exportParametersToClipboard(std::vector<ParameterGroup*> parameterGroups);
    }
}
