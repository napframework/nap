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
        lockComponent();
        getPatch().disconnect(source, destination);
        unlockComponent();
    }

    void PatchComponent::connect(nap::OutputPlugBase& source, nap::InputPlugBase& destination) {
        lockComponent();
        getPatch().connect(source, destination);
        unlockComponent();
    }

   
}
