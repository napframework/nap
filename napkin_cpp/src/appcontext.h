#pragma once

#include "actionstore.h"
#include "iconstore.h"
#include <QObject>
#include <QUndoCommand>
#include <QtWidgets/QMainWindow>
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/modulemanager.h>
#include <nap/xmlserializer.h>

#define UNTITLED_FILENAME "Untitled"


class AppContext : public QObject
{
	Q_OBJECT
private:
	AppContext();

public:
	void spawnWindow();
	QApplication* spawnWindowNonBlocking();

	void initialize();

	const QString& filename() const { return mFilename; }
	void newFile();
	void save();
	void save(const QString& filename);
	bool load(const QString& filename);
	bool isSaved() const;
	bool isDirty() const;
    void setClean();

    nap::Core& core();

	nap::Patch* activePatch() const { return mActivePatch; };
	void setActivePatch(nap::Patch* patch);

	// UNDO
	void undo() { mUndoStack.undo(); }
	void redo() { mUndoStack.redo(); }
	int undoIndex() const { return mUndoStack.index(); }
	int undoStackSize() const { return mUndoStack.count(); }
	void setUndoIndex(int index) { mUndoStack.setIndex(index); }
    QUndoStack& undoStack() { return mUndoStack; }
	const QUndoCommand* undoCommand(int index) const { return mUndoStack.command(index); }

	// SELECTION
	void setSelection(const QList<nap::Object*>& objects);
	const QList<nap::Object*>& selection() { return mSelection; }

	nap::Object* selectedObject() const
	{
		if (mSelection.size() > 0) return mSelection[0];
		return nullptr;
	}

	template <typename T>
	T* selected() const
	{
		for (nap::Object* obj : mSelection) {
			if (obj->getTypeInfo().isKindOf<T>()) return static_cast<T*>(obj);
		}
		return nullptr;
	}

	// SERIALIZATION
	QString serialize(nap::Object* obj);
	void deserialize(const QString& data, nap::Object* parent);

	void execute(QUndoCommand* cmd);

	IconStore& iconStore();
	ActionStore& actionStore();

signals:
	void selectionChanged(const QList<nap::Object*> objects);
	void undoChanged(int index);
	void sceneChanged();

private:
	QUndoStack mUndoStack;
	nap::Core mCore;
	nap::Patch* mActivePatch = nullptr;
	QString mFilename;
	std::unique_ptr<IconStore> mIconStore = nullptr;
	std::unique_ptr<ActionStore> mActionStore = nullptr;
	QList<nap::Object*> mSelection;


	//////////////////////////////
	// Singleton boilerplate below
	//////////////////////////////
public:
	static AppContext& get()
	{
		static AppContext instance;
		return instance;
	}

public:
	AppContext(AppContext const&) = delete;
	void operator=(AppContext const&) = delete;
};
