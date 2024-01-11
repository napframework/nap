#pragma once

#include <component.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parametersimple.h>
#include <parametercolor.h>
#include <uniforminstance.h>

namespace nap
{
	class UpdateMaterialComponentInstance;
	class RenderableMeshComponentInstance;

	/**
	 *	UpdateMaterialComponent
	 */
	class NAPAPI UpdateMaterialComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UpdateMaterialComponent, UpdateMaterialComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterRGBColorFloat>		mColor;
		ResourcePtr<ParameterVec2>				mFresnel;
	};


	/**
	 * UpdateMaterialComponentInstance	
	 */
	class NAPAPI UpdateMaterialComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateMaterialComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize UpdateMaterialComponentInstance based on the UpdateMaterialComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the UpdateMaterialComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		Slot<RGBColorFloat>	mColorChangedSlot;
		Slot<glm::vec2>	mFresnelChangedSlot;

		template<typename T>
		void onUniformValueUpdate(T value, TypedUniformValueInstance<T>* uniformInstance)
		{
			assert(uniformInstance != nullptr);
			uniformInstance->setValue(value);
		}

		void onUniformRGBColorUpdate(RGBColorFloat value, UniformVec3Instance* uniformInstance)
		{
			assert(uniformInstance != nullptr);
			uniformInstance->setValue(value.toVec3());
		}
	};
}
