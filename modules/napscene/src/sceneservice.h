#pragma once

#include "entity.h"
#include <nap/service.h>

namespace nap
{
	class NAPAPI SceneService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		using SceneSet = std::unordered_set<Scene*>;

        // Default Constructor
		SceneService() = default;

		// Default Destructor
		virtual ~SceneService() = default;

		/**
		 * @return All scenes that are loaded.
		 */
		const SceneSet& getScenes() const { return mScenes; }

	protected:

		/**
		* Object creation registration
		*/
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * Updates all scenes.
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Recursively updates the transform hierarchy for all scenes
		 */
		virtual void postUpdate(double deltaTime) override;

	private:
		friend class Scene;

		/**
		 * Registers scene into service. Called by Scene ctor.
		 */
		void registerScene(Scene& scene);

		/**
		 * Unregisters scene from service. Called by scene dtor.
		 */
		void unregisterScene(Scene& scene);

	private:
		SceneSet mScenes;		//< All loaded scenes
	};
}
