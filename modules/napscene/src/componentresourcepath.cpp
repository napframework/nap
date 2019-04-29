#include "componentresourcepath.h"
#include "entity.h"

namespace nap
{
	ComponentResourcePath::ComponentResourcePath(const Entity& root) :
		mRoot(&root)
	{
	}


	void ComponentResourcePath::push(int childIndex)
	{
		mPath.push_back(childIndex);
	}


	void ComponentResourcePath::pushComponent(const std::string& component)
	{
		assert(mCurrentComponent.empty());
		mCurrentComponent = component;
	}


	void ComponentResourcePath::pop()
	{
		mPath.pop_back();
	}


	void ComponentResourcePath::popComponent()
	{
		assert(!mCurrentComponent.empty());
		mCurrentComponent = std::string();
	}


	bool ComponentResourcePath::fromString(const Entity& root, const std::string& path, ComponentResourcePath& resolvedPath, utility::ErrorState& errorState)
	{
		// Split the path into its components
		std::vector<std::string> path_components;
		utility::splitString(path, '/', path_components);

		const Entity* current_entity = nullptr;
		const std::string& root_element = path_components[0];

		// The path is always a path relative to the root. Every path needs to start with './', which is a stylistic choice to make the path syntactically the same
		// as normal ComponentPtr paths. However, we do not support '..' or absolute paths, as we cannot extend beyond the entity hierarchy for this root entity.
		if (!errorState.check(root_element == ".", "The instance property path %s for root entity %s needs to start with './'", path.c_str(), root.mID.c_str()))
			return false;

		resolvedPath = ComponentResourcePath(root);

		current_entity = &root;
	
		// Now resolve the rest of the path. Note that we iterate from the second element (because we've already processed the root) to the second-to-last element (because the last element specifies the component we're looking for )
		for (int index = 1; index < path_components.size() - 1; ++index)
		{
			const std::string& part = path_components[index];

			if (!errorState.check(part != "..", "Error parsing instance property path %s for object %s. Using '..' to indicate a relative path is not supported. Always specify a path in the form './EntityA/EntityB/Component", path.c_str(), root.mID.c_str()))
				return false;

			if (!errorState.check(part != ".", "Error parsing instance property path %s for object %s. Using '.' is only allowed as an indicator for the root element. Always specify a path in the form './EntityA/EntityB/Component", path.c_str(), root.mID.c_str()))
				return false;

			// Split the child specifier on ':'. Note that the ':' is optional and is only used to disambguate between multiple children
			std::vector<std::string> element_parts;
			utility::splitString(part, ':', element_parts);
			if (!errorState.check(element_parts.size() <= 2, "Error resolving ComponentPtr with path %s: path contains a child specifier with an invalid format (multiple colons found)", path.c_str()))
				return false;

			// Find all child entities matching the ID
			struct MatchingChild
			{
				const Entity*	mEntity;
				int				mChildIndex;
			};
			std::vector<MatchingChild> matching_children;
			const Entity::EntityList& children = current_entity->mChildren;
			for (int child_index = 0; child_index != children.size(); ++child_index)
			{
				const Entity* child = children[child_index].get();
				if (child->mID == element_parts[0])
					matching_children.push_back({ child, child_index });
			}

			// There must be at least one match
			if (!errorState.check(matching_children.size() != 0, "Error resolving ComponentPtr with path %s: child with ID '%s' not found in entity with ID '%s'", path.c_str(), element_parts[0].c_str(), current_entity->mID.c_str()))
				return false;

			// If the child specifier was a single ID, there must be only a single match and we set that entity as the new current entity
			if (element_parts.size() == 1)
			{
				if (!errorState.check(matching_children.size() == 1, "Error resolving ComponentPtr with path %s: path is ambiguous; found %d children with ID '%s' in entity with ID '%s'. Use the child specifier syntax 'child_id:child_index' to disambiguate.", path.c_str(), matching_children.size(), element_parts[0].c_str(), current_entity->mID.c_str()))
					return false;

				current_entity = matching_children[0].mEntity;
				resolvedPath.push(matching_children[0].mChildIndex);
			}
			else
			{
				// The child specifier contained an index to disambiguate between multiple children with the same ID; parse the index
				int array_index;
				if (!errorState.check(sscanf(element_parts[1].c_str(), "%d", &array_index) == 1, "Error resolving ComponentPtr with path %s: path contains a child specifier with an invalid format (unable to parse int from %s)", path.c_str(), element_parts[1].c_str()))
					return false;

				if (!errorState.check(array_index < matching_children.size(), "Error resolving ComponentPtr with path %s: path contains an invalid child specifier; found %d eligible children but index %d is out of range", path.c_str(), matching_children.size(), array_index))
					return false;

				// Use the child with the specified index as current entity
				current_entity = matching_children[array_index].mEntity;
				resolvedPath.push(matching_children[array_index].mChildIndex);
			}
		}

		// Now that we've gone through the path, we know the current entity must contain a component with an ID equal to the last element on the path. We look for it here.
		for (const rtti::ObjectPtr<Component>& component : current_entity->mComponents)
		{
			if (component->mID == path_components.back())
			{
				resolvedPath.mCurrentComponent = component->mID;
				return true;
			}
		}

		errorState.fail("Error resolving ComponentPtr with path %s: component %s not found in entity", path.c_str(), path_components.back().c_str());
		return false;
	}


	bool ComponentResourcePath::operator==(const ComponentResourcePath& other) const
	{
		if (mRoot != other.mRoot || mPath.size() != other.mPath.size() || mCurrentComponent != other.mCurrentComponent)
			return false;

		for (int i = 0; i != mPath.size(); ++i)
			if (mPath[i] != other.mPath[i])
				return false;

		return true;
	}
}