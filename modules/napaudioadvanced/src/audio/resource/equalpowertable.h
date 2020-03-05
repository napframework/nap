#pragma once

// Nap includes
#include <nap/resource.h>

// Audio includes
#include <audio/utility/translator.h>
#include <audio/utility/safeptr.h>
#include <audio/service/audioservice.h>

namespace nap
{

    namespace audio
    {

        /**
         * Resource managing an equal power lookup table that can be shared across multiple audio processes to save memory usage.
         */
        class NAPAPI EqualPowerTable : public Resource
        {
            RTTI_ENABLE(Resource)

        public:
            EqualPowerTable(NodeManager& nodeManager) : mNodeManager(&nodeManager) {  }

            // Inherited from Resource
            bool init(utility::ErrorState& errorState) override;

            float translate(const float& inputValue) { return mTable->translate(inputValue); }
            audio::SafePtr<EqualPowerTranslator<float>> getTable() { return mTable.get(); }

            int mSize = 256; // Property: 'Size' Size of the table in floats.

        private:
            NodeManager* mNodeManager = nullptr;
            audio::SafeOwner<EqualPowerTranslator<float>> mTable = nullptr;
        };


        using EqualPowerTableObjectCreator = rtti::ObjectCreator<EqualPowerTable, NodeManager>;

    }

}



