#pragma once

#include <audio/core/audionode.h>

namespace nap
{

    namespace audio
    {
        
        class NAPAPI BufferNode : public Node
        {
        public:
            BufferNode(NodeManager& nodeManager);
            
            InputPin audioInput = { this };
            OutputPin audioOutput = { this };
            
            void update();

        private:
            void bufferSizeChanged(int size) override;
            
            std::size_t mCacheSize = 0;
        };
        
        
        class NAPAPI BufferUpdateProcess : public Process
        {
        public:
            BufferUpdateProcess(NodeManager& nodeManager) : Process(nodeManager) { }
            
            void process() override;
            
            void registerBuffer(BufferNode*);
            void unregisterBuffer(BufferNode*);
            
        private:
            std::vector<BufferNode*> mBuffers;
        };
        
    }
    
}
