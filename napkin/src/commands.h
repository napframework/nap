#pragma once

#include <rtti/objectptr.h>
#include <rtti/rttipath.h>

#include <QUndoCommand>
#include <QtCore/QVariant>
#include <generic/propertypath.h>
#include <scene.h>

#include "typeconversion.h"

namespace napkin
{
    /**
     * TODO: To be implemented
     */
	class AddObjectCommand : public QUndoCommand
	{
	public:
		AddObjectCommand(const rttr::type& type, nap::rtti::RTTIObject* parent = nullptr);
        /**
         * Redo
         */
		void redo() override;

		/**
		 * Undo
		 */
		void undo() override;
	private:
		const rttr::type mType;
		std::string mObjectName;
		std::string mParentName = "";
	};

    /**
     * TODO: To be implemented
     */
	class DeleteObjectCommand : public QUndoCommand
	{
	public:
		DeleteObjectCommand(nap::rtti::RTTIObject& object);
        /**
         * Undo
         */
        void undo() override;

        /**
         * Redo
         */
        void redo() override;
	private:
		const std::string mObjectName;

	};

    /**
     * This command sets the value of a property
     * TODO: This will just set the value, undo cannot be currently made to work with nap.
     */
	class SetValueCommand : public QUndoCommand
	{
	public:
        /**
         * @param ptr The pointer to the object
         * @param path The path to the property
         * @param newValue The new value of the property
         */
		SetValueCommand(const PropertyPath& propPath, QVariant newValue);

        /**
         * Undo
         */
        void undo() override;

        /**
         * Redo
         */
        void redo() override;

	private:
		const PropertyPath mPath; // The path to the property
		QVariant mNewValue; // The new value
		QVariant mOldValue; // The old value
	};

	class SetPointerValueCommand : public QUndoCommand
	{
	public:
        /**
         * @param ptr The pointer to the object
         * @param path The path to the property
         * @param newValue The new value of the property
         */
		SetPointerValueCommand(const PropertyPath& path, nap::rtti::RTTIObject* newValue);

        /**
         * Undo
         */
        void undo() override;

        /**
         * Redo
         */
        void redo() override;

	private:
		const PropertyPath	mPath;		// The path to the property
		const std::string	mNewValue;	// The new value
		std::string	mOldValue;	// The old value
	};


	/**
	 * TODO: Can this be an 'AddPointerToVectorCommand'?
	 * Add an entity to a scene
	 */
	class AddEntityToSceneCommand : public QUndoCommand
	{
	public:
		AddEntityToSceneCommand(nap::Scene& scene, nap::Entity& entity);

		void redo() override;
		void undo() override;
	private:
		const std::string mSceneID;
		const std::string mEntityID;
		size_t mIndex;
	};


	/**
	 * Add an element to an array
	 */
	class ArrayAddValueCommand : public QUndoCommand
	{
	public:
		/**
		 * @param prop The array property to add the element to
		 * @param index The index at which to insert the value
		 */
		ArrayAddValueCommand(const PropertyPath& prop, size_t index);

		/**
		 * @param prop The array property to add the element to
		 */
		ArrayAddValueCommand(const PropertyPath& prop);

		void redo() override;
		void undo() override;
	private:
		const PropertyPath& mPath; ///< The path to the array property
		size_t mIndex; ///< The index of the newly created element
	};

	/**
	 * Add an element to an array
	 */
	class ArrayAddNewObjectCommand : public QUndoCommand
	{
	public:
		/**
		 * @param prop The array property to add the element to
		 */
		ArrayAddNewObjectCommand(const PropertyPath& prop, const nap::rtti::TypeInfo& type, size_t index);

		/**
		 * @param prop The array property to add the element to
		 * @param index The index at which to insert the value
		 */
		ArrayAddNewObjectCommand(const PropertyPath& prop, const nap::rtti::TypeInfo& type);

		void redo() override;
		void undo() override;
	private:
		const PropertyPath& mPath; ///< The path to the array property
		nap::rtti::TypeInfo mType; ///< The type of object to create
		size_t mIndex; ///< The index of the newly created element
	};


	/**
	 * Add an existing element to an array
	 */
	class ArrayAddExistingObjectCommand : public QUndoCommand
	{
	public:
		/**
		 * @param prop The array property to add the element to
		 */
		ArrayAddExistingObjectCommand(const PropertyPath& prop, nap::rtti::RTTIObject& object, size_t index);

		/**
		 * @param prop The array property to add the element to
		 * @param index The index at which to insert the element
		 */
		ArrayAddExistingObjectCommand(const PropertyPath& prop, nap::rtti::RTTIObject& object);

		void redo() override;
		void undo() override;
	private:
		const PropertyPath& mPath; ///< The path to the array property
		const std::string mObjectName; ///< The type of object to create
		size_t mIndex; ///< The index of the newly created element
	};

	/**
	 * Remove an element from an array at the specified index
	 */
	class ArrayRemoveElementCommand : public QUndoCommand
	{
	public:
		/**
		 * @param array_prop The property representing the array
		 * @param index The index of the element to remove
		 */
		ArrayRemoveElementCommand(const PropertyPath& array_prop, size_t index);

		void redo() override;
		void undo() override;
	private:
		const PropertyPath& mPath; ///< The path representing the array
		nap::rtti::Variant mValue;
		size_t mIndex; ///< The element to be removed
	};

	class ArrayMoveElementCommand : public QUndoCommand
	{
	public:
		/**
		 * Reorder an element within an array
		 * @param array_prop The array that contains the element
		 * @param fromIndex The index of the element to move
		 * @param toIndex The index at which the element must be after the move
		 */
		ArrayMoveElementCommand(const PropertyPath& array_prop, size_t fromIndex, size_t toIndex);

		void redo() override;
		void undo() override;
	private:
		const PropertyPath& mPath; ///< The path representing the array
		size_t mFromIndex; ///< The element index to move
		size_t mToIndex; ///< The element index to move to
		size_t mOldIndex; ///< The actual old index (may have been shifted)
		size_t mNewIndex; ///< The actual new index (may have been shifted)
	};

};
