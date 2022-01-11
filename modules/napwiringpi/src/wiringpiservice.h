/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <set>

// Nap includes
#include <nap/service.h>
#include <concurrentqueue.h>

namespace nap {
    class NAPAPI WiringPiService : public nap::Service
    {
        RTTI_ENABLE(nap::Service)

    public:
        WiringPiService(ServiceConfiguration* configuration);
        
        // Initialization
        bool init(nap::utility::ErrorState& errorState) override;
    protected:
        void registerObjectCreators(rtti::Factory& factory) override final;

        void update(double deltaTime) override;

    private:
    };
        
}
