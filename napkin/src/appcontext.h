#pragma once

#include <QObject>

#include <nap/core.h>
#include <nap/entity.h>
#include <rtti/rttireader.h>
#include <QtWidgets/QUndoCommand>
#include <QtWidgets/QApplication>

#define WIN_GEO "windowGeometry"
#define WIN_STATE "windowState"
#define LAST_OPENED_FILE "lastOpenedFile"
#define NAP_FILE_FILTER "NAP JSON File (*.nap.json, *.json)"


class AppContext : public QObject {
Q_OBJECT

public:
    static AppContext& get();
    AppContext(AppContext const&) = delete;
    void operator=(AppContext const&) = delete;

    ~AppContext() override {}

    nap::Core& core() { return mCore; }

    void newFile();
    void loadFile(const QString& filename);
    void saveFile();
    void saveFileAs(const QString& filename);
    const QString lastOpenedFilename();
    const QString& currentFilename() { return mCurrentFilename; }

    nap::rtti::OwnedObjectList& objects() { return mObjects; }
    nap::rtti::RTTIObject* getObject(const std::string& name);

    // TODO: Data structure operations should be accessed on the objects themselves
    nap::Entity* getParent(const nap::Entity& entity);
    nap::Entity* getOwner(const nap::Component& component);
    nap::Entity* createEntity(nap::Entity* parent = nullptr);
    nap::Component* addComponent(nap::Entity& entity, rttr::type type);
    nap::rtti::RTTIObject* addObject(rttr::type type);
    void deleteObject(nap::rtti::RTTIObject& object);

    QStringList availableThemes();
    void setTheme(const QString& themeName);
    QString themeDir();

    void executeCommand(QUndoCommand* cmd);
    QApplication* qApplication() { return dynamic_cast<QApplication*>(qGuiApp);}

    QUndoStack& undoStack() { return mUndoStack; }

signals:
    void fileOpened(const QString& filename);
    void fileSaved(const QString& filename);
    void newFileCreated();

    void selectionChanged();

    // All data could have changed.
    void entityAdded(nap::Entity* newEntity, nap::Entity* parent=nullptr);
    void componentAdded(nap::Component& comp, nap::Entity& owner);
    void objectAdded(nap::rtti::RTTIObject& obj);
    void objectRemoved(nap::rtti::RTTIObject& object);
private:
    AppContext();
    std::string getUniqueName(const std::string& suggestedName);

    std::vector<std::unique_ptr<nap::rtti::RTTIObject>> mObjects;
    QUndoStack mUndoStack;
    QString mCurrentFilename;
    nap::Core mCore;

};