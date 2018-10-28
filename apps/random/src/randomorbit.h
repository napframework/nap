#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <rtti/objectptr.h>
#include <entity.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	// Forward Declares
	class RandomApp;

	/**
	* Handles all orbit related functionality for the random app
	*/
	class RandomOrbit final
	{
	public:
		/**
		* Constructor, needs the random app to function
		*/
		RandomOrbit(RandomApp& app);

		/**
		*	Update orbit components after properties changed
		*/
		void update(double deltaTime);
		void updateOrbit();

		/**
		* Add Renderable Components to provided vector
		*/
		void appendRenderableComponents(std::vector<nap::RenderableComponentInstance*>& renderable_components);

		/**
		*	Exposed properties for GUI
		*/
		float		getAngle();
		float		mRadius = 0.874f;
		const float mRadiusMin = 0.5f;
		const float mRadiusMax = 1.5f;
		float		mProgress = 0.0f;
		float		mCenter[2] = { -0.24f, -0.83f };
		const float mCenterRange = 1.5f;
		float		mStartEnd[2] = { 40.0f, 115.0f };

	private:
		// Initialized Variables
		const glm::vec2					mUvOffset = glm::vec2(6.0f, 2.0f);
		const float						mUvScale = 136.0f;

		// Scene Objects
		rtti::ObjectPtr<EntityInstance>	mOrbit = nullptr;
		rtti::ObjectPtr<EntityInstance>	mOrbitPath = nullptr;
		rtti::ObjectPtr<EntityInstance>	mOrbitStart = nullptr;
		rtti::ObjectPtr<EntityInstance>	mOrbitEnd = nullptr;
		rtti::ObjectPtr<EntityInstance>	mOrbitSun = nullptr;

		// The app used to extract information from
		RandomApp&	mApp;
	};
}
