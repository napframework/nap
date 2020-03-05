#pragma once

#include <audio/core/audionode.h>


namespace nap {
    
    namespace audio {
        
        class NAPAPI GenericNode : public Node {
            RTTI_ENABLE(nap::audio::Node)
        public:
            GenericNode(NodeManager& manager) : Node(manager) { }
            
            int getInputCount() const { return mInputs.size(); }
            int getOutputCount() const { return mOutputs.size(); }
            bool hasParameter(const std::string& name) const { return mParameters.find(name) != mParameters.end(); }
            std::vector<std::string> getParameterNames() const;
            
            InputPin* getInput(unsigned int index) { return mInputs[index].get(); }
            OutputPin* getOutput(unsigned int index) { return mOutputs[index].get(); }
            float* getParameterValue(const std::string& name) const;
            bool setParameterValue(const std::string& name, float value);
            bool setParameterNormalizedValue(const std::string& name, float value);
            
            void addParameter(const std::string& name, float* zone, float min, float max);
            
        protected:
            void setInputCount(unsigned int count);
            void setOutputCount(unsigned int count);
            
        private:
            class Parameter {
            public:
                float* mZone = nullptr;
                float mMin = 0.f;
                float mMax = 1.f;
            };
            
            std::vector<std::unique_ptr<InputPin>> mInputs;
            std::vector<std::unique_ptr<OutputPin>> mOutputs;
            std::map<std::string, Parameter> mParameters;
        };
        
    }
    
}
