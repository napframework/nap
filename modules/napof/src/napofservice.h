#pragma once

// Nap Includes
#include <nap/service.h>
#include <napofrendercomponent.h>
#include <napoftransform.h>
#include <napofmaterial.h>
#include <nap/signalslot.h>
#include <napofsimplecamcomponent.h>
#include <nap/configure.h>

// Std Includes
#include <vector>

// RTTI Includes
#include <rtti/rtti.h>

// Forward Declares
namespace nap
{
	class Entity;
}


namespace nap
{
	/**
	@brief nap openframeworks service

	Registers all the OF defined operators / components and other data types
	**/
	class OFService : public Service
	{
		// RTTI Derived from Service
		RTTI_ENABLE_DERIVED_FROM(Service)

		// Declare the service
		NAP_DECLARE_SERVICE()

	public:
		// Display list update mode
		enum class DisplayMode : nap::int8
		{
			ALWAYS,			//< Updates the display list every frame
			LAZY,			//< Updates the display list on registration / de-registration of components
			DISABLED		//< Does not update the display list, update list manual
		};

		// Constructor
		OFService() = default;

		// Draw
		void draw();

		// Update
		void update();

		// Default cam (perspective drawing)
		void setDefaultCamera(Entity& inEntity);
		void removeDefaultCamera();
		bool hasDefaultCamera() const				{ return mDefaultCamera != nullptr; }

		// Filters
		static bool sObjectIsDrawable(Object& inComponent, Core& inCore);
		static bool sHasTopXform(Object& inComponent, Core& inCore);

		// Display List
		void updateDisplayList();
		void setDisplayMode(DisplayMode inMode);

	protected:
		virtual void objectRegistered(Object& inObject) override;
		virtual void objectRemoved(Object& inObject);

	private:
		// Defines
		using DisplayList = std::vector<OFRenderableComponent*>;
		using TransformList = std::vector<OFTransform*>;
		using DisplayMap = std::unordered_map<OFSimpleCamComponent*, vector<OFRenderableComponent*>>;
		using XFormList = std::vector<OFTransform*>;

		// Utility
		void drawComponents(DisplayList& inComponentsToDraw);
		void transform(const OFTransform& inTransform);
		void setIsDirty(const Object& inObject);
		void sortBasedOnDistance(const ofVec3f& inPos, DisplayList& inItems);
		
		// Dirty flags for optimized updating of transforms and display lists
		bool isDirtyDrawing = true;
		bool isDirtyXform = true;

		// Default cam
		Entity* mDefaultCamera = nullptr;

		// Display list
		DisplayMap	mBufferDisplayList;				//< Holds all buffer related display lists
		DisplayList mDefaultDisplayList;			//< Holds the default display list for main camera
		DisplayMode mMode = DisplayMode::LAZY;		//< Lazy display list update

		// Top Transform List
		XFormList	mTopXforms;

		// When the drawing mode changes update display list
		void visibilityChanged(const bool& inValue);
		NSLOT(mDisplayChanged, const bool&, visibilityChanged)
	};
}

// Declare service
RTTI_DECLARE(nap::OFService)
