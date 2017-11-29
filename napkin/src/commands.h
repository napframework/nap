#pragma once

#include <nap/objectptr.h>
#include <rtti/rttipath.h>

#include <QUndoCommand>
#include <QtCore/QVariant>

#include "typeconversion.h"
namespace napkin
{
    /**
     * TODO: To be implemented
     */
	class AddObjectCommand : public QUndoCommand
	{
	public:
        /**
         * Undo
         */
		void undo() override;

        /**
         * Redo
         */
		void redo() override;
	};

    /**
     * TODO: To be implemented
     */
	class DeleteObjectCommand : public QUndoCommand
	{
	public:
        /**
         * Undo
         */
        void undo() override;

        /**
         * Redo
         */
        void redo() override;
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
		SetValueCommand(nap::rtti::RTTIObject* ptr, nap::rtti::RTTIPath path, QVariant newValue);

        /**
         * Undo
         */
        void undo() override;

        /**
         * Redo
         */
        void redo() override;

	private:
		nap::rtti::RTTIObject* mObject; // The object that has the property
		nap::rtti::RTTIPath mPath; // The path to the property
		QVariant mNewValue; // The new value
		QVariant mOldValue; // The old value
	};
};