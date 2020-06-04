#pragma once

// Nap includes
#include <parametervec.h>
#include <parameternumeric.h>
#include <parametersimple.h>
#include <component.h>
#include <nap/signalslot.h>
#include <nap/resourceptr.h>


// Forward declare
namespace casipan { struct GridSettings; }

namespace nap
{

    // Forward declares
    class ParameterComponentInstance;
    class Parameter;

    namespace spatial
    {
        

        /**
         * A set of all grid-specific parameters for 4dpan.
         */
        class NAPAPI GridSettingsParams {
            
        public:
            GridSettingsParams(ParameterComponentInstance* parameterComponent, const std::string& name, casipan::GridSettings* gridSettings);
            
            std::string mName;

            Signal<> mDataChanged; ///< signal emitted when some data changed.
            
        private:
            ParameterFloat* mGainLevel = nullptr;
            ParameterFloat* mBoostLevel = nullptr;
            ParameterFloat* mCurvature = nullptr;
            ParameterVec3* mProjectionPoint = nullptr;
            ParameterBool* mOrthogonalProjection = nullptr;
            ParameterBool* mDistanceAttenuation = nullptr;
            ParameterFloat* mDistanceAttenuationThreshold = nullptr;
            ParameterFloat* mDistanceAttenuationCurvature = nullptr;
            
            casipan::GridSettings* mGridSettings; ///< Pointer to GridSettings object where the data needs to be stored.
        };
        
        
        class GridSettingsComponentInstance;
        class MultiSpeakerSetup;
        
        /**
         * The GridSettingsComponent manages the 4dpan parameters per MultiSpeakerSetup-grid for a soundobject.
         */
        class NAPAPI GridSettingsComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(GridSettingsComponent, GridSettingsComponentInstance)
            
        public:
            GridSettingsComponent() : Component() { }
            
            virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
            
            /** 
             * Property. Pointer to the @MultiSpeakerSetup.
             */
            ResourcePtr<MultiSpeakerSetup> mMultiSpeakerSetup;
            
        };

        /**
         * The GridSettingsComponent manages the 4dpan parameters per MultiSpeakerSetup-grid for a soundobject.
         */
        class NAPAPI GridSettingsComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
        public:
            GridSettingsComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
            
            /** 
             * Initialises the component. Creates a @GridSettingsParams instance per grid of the MultiSpeakerSetup.
             */
            bool init(utility::ErrorState& errorState) override;
            
            
            /**
             * Returns signal emitted when the grid settings have changed.
             */
            Signal<const GridSettingsComponentInstance&>* getGridSettingsChanged(){
                return &gridSettingsChanged;
            }
            
            /**
             * Returns a vector of 4dpan GridSettings to be used in the panning call of MultiSpeakerSetup.
             */
            std::vector<casipan::GridSettings>& getGridSettings(){
                return mGridSettings;
            }

        private:
            Signal<const GridSettingsComponentInstance&> gridSettingsChanged;

            
            std::vector<casipan::GridSettings> mGridSettings; ///< The data stored in a 4dpan 'GridSettings' struct. This vector doesn't get resized after init.
            std::vector<std::unique_ptr<GridSettingsParams>> mGridParams; ///< The parameters.
            
            void triggerGridSettingsChanged(){ gridSettingsChanged.trigger(*this); }

        };
           
    }
}
