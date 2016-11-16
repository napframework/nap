//
//  patchcomponent.cpp
//  Project
//
//  Created by Stijn van Beek on 3/9/16.
//
//

#include "patchcomponent.h"

RTTI_DEFINE(nap::PatchComponent)

using namespace std;

namespace nap
{

    void PatchComponent::disconnnect(nap::OutputPlugBase& source, nap::InputPlugBase& destination) {
        getPatch().disconnect(source, destination);
    }

    void PatchComponent::connect(nap::OutputPlugBase& source, nap::InputPlugBase& destination) {
        getPatch().connect(source, destination);
    }

   
}
