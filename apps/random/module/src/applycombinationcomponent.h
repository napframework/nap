#pragma once

#include <component.h>
#include <bitmap.h>
#include <artnetmeshfromfile.h>

namespace nap
{
	class ApplyCombinationComponentInstance;

	/**
	 * This component applies the rendered effect (in the combination texture) to the mesh as a color value
	 * Where Red is the first LED and Green is the second LED. The result can be rendered to screen and 
	 * used to send the color information as artnet packages over the network
	 */
	class NAPAPI ApplyCombinationComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ApplyCombinationComponent, ApplyCombinationComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		rtti::ObjectPtr<Bitmap> mBitmap = nullptr;				///< Property: 'Bitmap' the bitmap that contains the color information for the mesh
		rtti::ObjectPtr<ArtnetMeshFromFile> mMesh = nullptr;	///< Property: 'Mesh' the mesh that the bitmap is applied to
		float mBrightness = 1.0f;								///< Property: 'Brightness' Overall LED brightness
		float mInfluence = 1.0f;								///< Property: 'EffectInfluence' Influence of combination effect on led brightness
	};


	/**
	 * applycombinationcomponentInstance	
	 */
	class NAPAPI ApplyCombinationComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ApplyCombinationComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize applycombinationcomponentInstance based on the applycombinationcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the applycombinationcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update applycombinationcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		ArtnetMeshFromFile* mMesh = nullptr;
		Bitmap* mBitmap = nullptr;
		float mBrightness = 1.0f;
		float mInfluence = 1.0f;
	};
}
