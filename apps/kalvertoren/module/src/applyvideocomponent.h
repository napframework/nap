#pragma once

#include "applycolorcomponent.h"
#include "compositioncomponent.h"
#include "colorpalettecomponent.h"
#include "rendercompositioncomponent.h"
#include "ledcolorpalettegrid.h"
#include "rendervideocomponent.h"
#include "videocontrolcomponent.h"
#include "indexmap.h"

#include <component.h>
#include <componentptr.h>
#include <triangleiterator.h>

namespace nap
{
	class ApplyVideoComponentInstance;

	/**
	 *	Applies a video to the mesh
	 */
	class NAPAPI ApplyVideoComponent : public ApplyColorComponent
	{
		RTTI_ENABLE(ApplyColorComponent)
		DECLARE_COMPONENT(ApplyVideoComponent, ApplyVideoComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<RenderVideoComponent>	mVideoRenderer;				///< property: link to the video render component
		ComponentPtr<VideoControlComponent>	mVideoController;			///< property: link to the video controller
		ResourcePtr<LedColorPaletteGrid> mColorGrid;					///< property: link to the color palette grid
		ResourcePtr<WeekColors> mVideoColors;							///< property: link to the specified video colors
		ResourcePtr<IndexMap> mIndexMap;								///< property: link to the index map
	};


	/**
	 * Applies a video to a mesh
	 */
	class NAPAPI ApplyVideoComponentInstance : public ApplyColorComponentInstance
	{
		RTTI_ENABLE(ApplyColorComponentInstance)
	public:
		ApplyVideoComponentInstance(EntityInstance& entity, Component& resource) :
			ApplyColorComponentInstance(entity, resource)										{ }

		/**
		 * Initialize ApplyVideoComponentInstance based on the ApplyColorComponentInstance resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the applycompositioncomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Applies bounding box colors to the mesh
		 */
		virtual void applyColor(double deltaTime) override;

		// Resolved pointer to the video renderer
		ComponentInstancePtr<RenderVideoComponent> mVideoRenderer =		{ this, &ApplyVideoComponent::mVideoRenderer };

		// Resolved pointer to the video controller
		ComponentInstancePtr<VideoControlComponent> mVideoController =	{ this, &ApplyVideoComponent::mVideoController };

	private:
		/**
		 * Builds a map that binds the index colors to the currently selected palette colors
		 * Note that when the index color count > palette color count the first palette color is used
		 */
		void updateSelectedPalette();

		/**
		* @param indexColor the color to get the associated palette color for
		* @return the palette color associated with a certain index map color
		*/
		LedColorPaletteGrid::PaletteColor getPaletteColor(const IndexMap::IndexColor& indexColor) const;

		// Map that binds index colors to current color palette colors
		std::map<IndexMap::IndexColor, LedColorPaletteGrid::PaletteColor> mIndexToPaletteMap;

		// Members from resource
		LedColorPaletteGrid* mColorGrid = nullptr;
		WeekColors* mVideoColors = nullptr;
		IndexMap* mIndexMap = nullptr;
	};
}
