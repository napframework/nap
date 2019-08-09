#pragma once

#include "utility/errorstate.h"
#include <string>
#include <vector>

namespace nap
{
	class Entity;

	/**
	 * Represents a path to a component in an entity hierarchy.
	 */
	class ComponentResourcePath
	{
	public:
		ComponentResourcePath() = default;

		ComponentResourcePath(const Entity& root);
		
		/**
		 * @return the root entity in the hierarchy.
		 */
		const Entity& getRoot() const { return *mRoot; }

		/**
		 * Pushes a child entity onto the path.
		 * @param childIndex The absolute index of this child in the entity child array of the parent.
		 */
		void push(int childIndex);

		/**
		 * Pops the last child entity from the path.
		 */
		void pop();

		/**
		 * Pushes the last item, the component, onto the path.
		 * @param component: ID of the component to push onto the path.
		 */
		void pushComponent(const std::string& component);
	
		/**
		 * Pops the component from the path.
		 */
		void popComponent();

		/**
		 * Creates an EntityResourcePath from a root entity and a string. 
		 * @param root The root entity that is used by path.
		 * @param path The string containing the path from root to the component. This path includes the name of the component.
		 * @param resolvedPath If the function returns true, this parameter contains the created EntityResourcePath.
		 * @param errorState contains error information on failure.
		 * @return true on success, false on failure. If the function return false, check errorState for information.
		 */
		static bool fromString(const Entity& root, const std::string& path, ComponentResourcePath& resolvedPath, utility::ErrorState& errorState);

		/**
		 * Compares on EntityResourcePath to another.
		 * @return True when equal, false otherwise.
		 */
		bool operator==(const ComponentResourcePath& other) const;

	private:
		const Entity*		mRoot = nullptr;		///< Root entity
		std::vector<int>	mPath;					///< Path from the root entity to the component
		std::string			mCurrentComponent;		///< ID of the component
	};
}
