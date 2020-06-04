#pragma once

// Audio includes
#include <audio/core/multichannel.h>
#include <audio/core/audioobject.h>

// Nap includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <entity.h>

// Macro to define an Effect named Type from instance InstanceType.
#define DECLARE_EFFECT(Type, InstanceType)  class NAPAPI Type : public Effect<InstanceType> { RTTI_ENABLE(EffectBase) };


namespace nap
{
    
    namespace audio
    {
        
        class AudioService;
    }
    
    namespace spatial
    {

        // Forward declarations
        class EffectProcessor;
        class EffectChainInstance;
        class EffectInstanceBase;

        /**
         * Base class for all @Effect resources.
         */
        class NAPAPI EffectBase : public Resource
        {
            RTTI_ENABLE(Resource)
            
        public:
            EffectBase() = default;
            
            std::string mName; ///< property: 'Name' Used to set parameter prefixes.

            /**
             * Instantiate an @EffectInstance from this resource.
             * @param channelCount Number of channels that the new instance will process.
             * @param audioService Audio service needed to spawn DSP nodes.
             * @param errorState
             * @param entity Optional pointer to an entity that the Effect resides in.
             * @return nullptr on failure.
             */
            virtual std::unique_ptr<EffectInstanceBase> instantiate(int channelCount, audio::AudioService& audioService, nap::utility::ErrorState& errorState, nap::EntityInstance* entity = nullptr) { return nullptr; }
        };

        
        /**
         * Base class for all effect instances.
         * @EffectChain holds a vector of EffectInstanceBase.
         */
        class NAPAPI EffectInstanceBase {
            RTTI_ENABLE()
            
        public:
            EffectInstanceBase(EffectBase& effect) : mName(effect.mName), mEffect(&effect) {};
            virtual ~EffectInstanceBase() = default;

            EffectInstanceBase(const EffectInstanceBase&) = delete;
            EffectInstanceBase& operator=(const EffectInstanceBase&) = delete;
            
            virtual bool addProcessor(audio::IMultiChannelOutput& input, utility::ErrorState& errorState) = 0;
            
            virtual EffectProcessor* getProcessorNonTyped(int index) = 0;
            
            /**
             * Virtual update function that can be implemented. Called by EffectChainInstance.
             * This function will only be getting called if the owner of the EffectChainInstance owning this EffectInstance is calling the EffectChainInstance::update() function in its update-loop
             */
            virtual void update(double deltaTime) { }
            
            
            const std::string& getName() const { return mName; }

        protected:
            template <typename T>
            T* getResource() { return rtti_cast<T>(mEffect); }
            
        private:
            std::string mName;
            EffectBase* mEffect = nullptr;
        };
        
        
        
        /**
         * An @Effect is a resource that can instantiate @EffectInstance instances.
         */
        template <typename InstanceType>
        class NAPAPI Effect : public EffectBase
        {
            RTTI_ENABLE(EffectBase)
            
        public:
            Effect() = default;
            
            /**
             * Instantiates and initialises an EffectInstance. Returns nullptr if initialisation failed.
             */
            std::unique_ptr<EffectInstanceBase> instantiate(int channelCount, audio::AudioService& audioService, nap::utility::ErrorState& errorState, nap::EntityInstance* entity = nullptr) override;
        };
        
        
        /**
         * Baseclass for all effect processor objects.
         * Very similar to an @AudioObjectInstance except that the @init() method has a fixes signature.
         */
        class NAPAPI EffectProcessor : public audio::IMultiChannelInput, public audio::IMultiChannelOutput
        {
            RTTI_ENABLE()
            
        public:
            EffectProcessor(EffectBase& effect) : mEffect(&effect) { }
            virtual ~EffectProcessor() = default;

            EffectProcessor(const EffectProcessor&) = delete;
            EffectProcessor& operator=(const EffectProcessor&) = delete;

            /**
             * Each @EffectProcessor has to implement this in order to be able to be spawned by an @EffectInstance.
             * @param audioService Used to create nodes.
             * @param channelCount Number of channels that the processor has to process.
             * @param errorState
             * @return True on success.
             */
            virtual bool init(audio::AudioService& audioService, int channelCount, utility::ErrorState& errorState) = 0;

        protected:
            template <typename T>
            T* getResource() { return rtti_cast<T>(mEffect); }

        private:
            EffectBase* mEffect = nullptr;
        };


        /**
         * An @EffectInstance is the instance of an @Effect. It creates parameters on init and creates and connects an @EffectProcessor for each processing chain.
         */
        template <typename ProcessorType>
        class NAPAPI EffectInstance : public EffectInstanceBase
        {
            RTTI_ENABLE()
            
        public:
            /**
             * Constructor. Sets the mChannelCount and mAudioService members.
             */
            EffectInstance(EffectBase& effect, audio::AudioService& audioService, int channelCount) : EffectInstanceBase(effect), mAudioService(audioService), mChannelCount(channelCount) { }
            virtual ~EffectInstance() { }
            
            /**
             * Should create parameters and get necessary input/particle transform pointers in entity.
             */
            virtual bool init(nap::utility::ErrorState& errorState, nap::EntityInstance* entity = nullptr) = 0;

            /**
             * Used by @EffectChainInstance to add processors to the chain.
             * @param input Interface to the previous processor.
             * @param errorState
             * @return true on success.
             */
            bool addProcessor(audio::IMultiChannelOutput& input, utility::ErrorState& errorState) override;

            /**
             * Used by @EffectInstance to iterate through its non typed processors.
             * @param index
             * @return EffectProcessor base class.
             */
            EffectProcessor* getProcessorNonTyped(int index) override
            {
                return mEffectProcessors[index].get();
            }

            /**
             * @return Return processor by index.
             */
            ProcessorType* getProcessor(int index)
            {
                return mEffectProcessors[index].get();
            }

            /**
             * @return Number of channels in the effect.
             */
            int getChannelCount() { return mChannelCount; }

            /**
             * @return Number of processors in the effect instance.
             */
            int getProcessorCount() { return mEffectProcessors.size(); }
            
        protected:
            audio::AudioService& getAudioService() { return mAudioService; }
            
            /**
             * Virtual function that is called directly after a processor has been added.
             */
            virtual void onProcessorAdded(ProcessorType& newProcessor) { }


        private:
            std::vector<std::unique_ptr<ProcessorType>> mEffectProcessors;
            audio::AudioService& mAudioService;
            int mChannelCount = 0;
        };
        
        
        /**
         * The @EffectChain is a resource that can instantiate @EffectChainInstance instances. It has a property with a vector of @Effect s.
         */
        class NAPAPI EffectChain : public Resource
        {
            RTTI_ENABLE(Resource)
        public:
            EffectChain() = default;
            
            std::vector<ResourcePtr<EffectBase>> mEffects; ///< Property.
            
            /**
             * Instantiates and initialises an @EffectChainInstance. Returns nullptr if initisialisation failed.
             */
            virtual std::unique_ptr<EffectChainInstance> instantiate(int channelCount, audio::AudioService& audioService, nap::utility::ErrorState& errorState, nap::EntityInstance* entity = nullptr);
        };


        /**
         * Instance of @EffectChain.
         */
        class NAPAPI EffectChainInstance
        {
            RTTI_ENABLE()
            
        public:
            
            /**
             * Default constructor because of NAP convention.. After the constructor, the init function has to be called.
             */
            EffectChainInstance() = default;

            virtual ~EffectChainInstance() = default;

			// Delete copy and move constructors.
			EffectChainInstance(const EffectChainInstance&) = delete;
			EffectChainInstance& operator=(const EffectChainInstance&) = delete;

            /**
             * Initialises @EffectInstance instances.
             */
            bool init(EffectChain* effectChain, int channelCount, audio::AudioService& audioService, nap::utility::ErrorState& errorState, nap::EntityInstance* entity = nullptr);
            
            /**
             * Updates all its EffectInstances. Should be called by the owner of this @EffectChainInstance.
             */
            virtual void update(double deltaTime);
            
            /**
             * Adds a processor to each @EffectInstance, and connects them sequentially to the given input.
             * Returns the output of the processor chain.
             */
            audio::IMultiChannelOutput* addProcessorChain(audio::IMultiChannelOutput& input, utility::ErrorState& errorState);
        
            /**
             * Returns the corresponding output of the last EffectInstance in the mEffectInstances vector. 
             */
            audio::IMultiChannelOutput& getChainOutput(int chainIndex);
            
            
            /**
             * Returns an effect by name. (To be used by ParticleVisualizationComponent to find the ResonanceEffect)
             */
            EffectInstanceBase* getEffectByName(std::string name);
            
            
        private:
            std::vector<std::unique_ptr<EffectInstanceBase>> mEffectInstances;
            std::vector<audio::IMultiChannelOutput*> mChainOutputs;
            int mProcessorCount = 0;
        };
        
        
        // --- SpatialAudio --- //
        
        template <typename InstanceType>
        std::unique_ptr<EffectInstanceBase> Effect<InstanceType>::instantiate(int channelCount, audio::AudioService& audioService, nap::utility::ErrorState& errorState, nap::EntityInstance* entity)
        {
            auto instance = std::make_unique<InstanceType>(*this, audioService, channelCount);
            bool initSucceeded = instance->init(errorState, entity);
            if(initSucceeded)
                return std::move(instance);
            else
                return nullptr;
        }


        template <typename ProcessorType>
        bool EffectInstance<ProcessorType>::addProcessor(audio::IMultiChannelOutput& input, utility::ErrorState& errorState)
        {
            auto processor = std::make_unique<ProcessorType>(*getResource<EffectBase>());

            bool initSucceeded = processor->init(mAudioService, mChannelCount, errorState);
            if (!initSucceeded)
            {
                errorState.fail("Failed to initialize effect processor.");
                return false;
            }

            assert(input.getChannelCount() == mChannelCount);
            assert(input.getChannelCount() == processor->getInputChannelCount());
            processor->IMultiChannelInput::connect(input);

            mEffectProcessors.emplace_back(std::move(processor));

            onProcessorAdded(*mEffectProcessors.back());

            return true;
        }





    }
}
