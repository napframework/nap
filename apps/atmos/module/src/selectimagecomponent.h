#pragma once

#include <component.h>
#include <imagefromfile.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

namespace nap
{
	class SelectImageComponentInstance;

	/**
	 *	selectimagecomponent
	 */
	class NAPAPI SelectImageComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectImageComponent, SelectImageComponentInstance)
	public:

		std::vector<ResourcePtr<ImageFromFile>> mImages;	///< Property: 'Images' list of selectable images
		ResourcePtr<ParameterInt> mIndex;					///< Property: 'Index' current selected image

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * selectimagecomponentInstance	
	 */
	class NAPAPI SelectImageComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SelectImageComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize selectimagecomponentInstance based on the selectimagecomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the selectimagecomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update selectimagecomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Select a new image
		 * @param index the new image index
		 */
		void selectImage(int index);
	
		/**
		 * @return the currently selected image
		 */
		ImageFromFile& getImage();
		
		/**
		 *	@return the currently selected image
		 */
		const ImageFromFile& getImage() const;

		/**
		* @return the number of selectable meshes
		*/
		int getCount() const				{ return mImages.size(); }

	private:
		std::vector<ImageFromFile*> mImages;						//< All Images to select from
		ImageFromFile* mCurrentImage = nullptr;						//< Current image
		nap::Slot<int> mImageIndexChangedSlot = { this, &SelectImageComponentInstance::selectImage };
	};
}
