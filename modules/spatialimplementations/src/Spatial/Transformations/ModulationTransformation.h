#pragma once

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Transformation/Transformation.h>

// Nap includes
#include <mathutils.h>

// Glm includes
#include <glm/gtc/noise.hpp>


namespace nap {
    namespace spatial {
        
        
        class ModulationTransformationInstance;
        
        /**
         * A class that defines a modulation on 1 axis. Manages a float value that's being modulated by an oscillator controlled by  parameters created by the Modulation class.
         * T should be a type that can add parameters.
         */
        class NAPAPI Modulation
        {
            
        public:
            /**
             * The constructor creates enable, mode, frequency, phase, power, depth & invert parameters.
             */
            Modulation(ParameterManager& parameterManager, std::string axisString)
            {
                mEnable = parameterManager.addParameterBool(axisString + "Enable", false);
                
                mMode = parameterManager.addParameterOptionList(axisString + "Waveform", "sine", {"sine","triangle","linear","random"});
                
                mFrequency = parameterManager.addParameterFloat(axisString + "Frequency", 1., 0., 100.);
                
                mPhase = parameterManager.addParameterFloat(axisString + "Phase", 0., 0., 1.);
                
                mPower = parameterManager.addParameterFloat(axisString + "Power", 1., 0., 10.);
                
                mDepth = parameterManager.addParameterFloat(axisString + "Depth", 1., 0., 100.);
                
                mInvert = parameterManager.addParameterBool(axisString + "Invert", false);
                
                mSeed = math::random<float>(0., 500.);

            }
            
            /**
             * Returns the current value.
             */
            float getValue()
            {
                return mValue;
            }
            
            /**
             * Updates the current value according to the current 'masterAccumulatedTime' (which is the total passed time taking into account  ModulationTransformations' "speed" parameter) and its parameters.
             */
            void update(double deltaTime, double masterAccumulatedTime)
            {
                
                mValue = 0;
                
                if(mEnable->mValue && mFrequency->mValue > 0.00001){
                    
                    // calculate phase (for sin, tri, lin) and time (for rand)
                    float period =  1. / mFrequency->mValue;
                    mAccumulatedTime += deltaTime * mFrequency->mValue;
                    float phase = fmodf(masterAccumulatedTime, period) / period;
                    
                    // phase
                    phase += mPhase->mValue;
                    if(phase > 1.)
                        phase -= 1.;
                    
                    // calculate value according to mode
                    int mode = mMode->getValue();
                    if(mode == 0){
                        mValue = sinf(2 * math::pi() * phase);
                    }
                    else if(mode == 1){
                        mValue = (1. - fabs(phase * 2. - 1.)) * 2. - 1.;
                    }
                    else if(mode == 2){
                        mValue = phase * 2. - 1.;
                    }
                    else if(mode == 3){
                        mValue = glm::simplex(glm::vec2(mAccumulatedTime, mSeed));
                    }
                    
                    // power
                    if(mPower->mValue >= 0){
                        if(mValue < 0)
                            mValue = -1 * powf(-mValue, mPower->mValue);
                        else
                            mValue = powf(mValue, mPower->mValue);
                    }
                    
                    // depth
                    mValue *= mDepth->mValue;
                    
                    // invert
                    if(mInvert->mValue)
                        mValue = -mValue;
                    
                }
                
            }
            
        private:
            float mAccumulatedTime = 0.;
            float mSeed = 0.;
            float mValue = 0.;
            
            ParameterBool* mEnable = nullptr;
            ParameterOptionList* mMode = nullptr;
            ParameterFloat* mFrequency = nullptr;
            ParameterFloat* mPhase = nullptr;
            ParameterFloat* mPower = nullptr;
            ParameterFloat* mDepth = nullptr;
            ParameterBool* mInvert = nullptr;
            
        };
        
        
        /**
         * Instance of @ModulationOffsetComponent.
         */
        class NAPAPI ModulationTransformationInstance : public TransformationInstance
        {
            RTTI_ENABLE(TransformationInstance)
            
            friend class Modulation;
            
        public:
            ModulationTransformationInstance(TransformationBase& transformation) : TransformationInstance(transformation) { }
            
            void onInit(nap::EntityInstance* entity) override;
            
            virtual void apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation) override;
            
            /**
             * Updates the accumulated time, updates the @Modulation instances for all axes and calculates the final offset.
             */
            virtual void update(double deltaTime) override;
            
        private:
            glm::vec3 mOffset = glm::vec3(0,0,0);
            float mAccumulatedTime = 0.;
            std::vector<Modulation> mModulations;
            
            ParameterFloat* mSpeed = nullptr;
            ParameterFloat* mScale = nullptr;
            ParameterBool* mEnable = nullptr;
            
        };
        
        DECLARE_TRANSFORMATION(ModulationTransformation, ModulationTransformationInstance)
        
    }
}
