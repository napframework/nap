#include "appcontext.h"
#include "commands.h"
#include "mainwindow.h"
#include <fstream>


void AppContext::spawnWindow()
{
	int argc = 1;
	char* argv[] = {strdup("Napkin")};
	QApplication* app = new QApplication(argc, argv);
	QApplication::setOrganizationName("NAP");
	QApplication::setApplicationName("Napkin");
	MainWindow w;
	w.show();
	AppContext::get().initialize();
	app->exec();
}

QApplication* AppContext::spawnWindowNonBlocking()
{
	int argc = 1;
	char* argv[] = {strdup("Napkin")};
	QApplication* app = new QApplication(argc, argv);
	QApplication::setOrganizationName("NAP");
	QApplication::setApplicationName("Napkin");
	MainWindow w;
	w.show();
	AppContext::get().initialize();
	return app;
}


AppContext::AppContext() {}

void AppContext::save() { save(mFilename); }

void AppContext::save(const QString& filename)
{
	FileType* fileType = fileTypeFromFilename(filename);
	if (!fileType) return;



	std::ofstream out;
	out.open(filename.toStdString().c_str(), std::ios_base::out);

	fileType->serialize(out, core(), core().getRoot());
	out.close();

	mFilename = filename;
	nap::Logger::info("File saved: %s", filename.toStdString().c_str());
	mUndoStack.setClean();
}

bool AppContext::load(const QString& filename)
{
	FileType* fileType = fileTypeFromFilename(filename);
	if (!fileType) return false;

	std::ifstream in;
	in.open(filename.toStdString().c_str(), std::ios_base::in);



	core().clear();

//	core().getModuleManager().loadModules();

	fileType->deserialize(in, core());

	return true;
}

void AppContext::initialize()
{
	connect(&mUndoStack, &QUndoStack::indexChanged, [&](int index) {
		undoChanged(index);
		sceneChanged();
	});
	connect(&mUndoStack, &QUndoStack::cleanChanged, [&](bool clean) {
//		undoChanged(-1);
		sceneChanged();
	});
	newFile();
	actionStore().updateStates();

	core().getModuleManager().loadModules();
}

void AppContext::newFile()
{
	mFilename = UNTITLED_FILENAME;
	core().clear();
	mUndoStack.setClean();
}


nap::Core& AppContext::core() { return mCore; }

void AppContext::setSelection(const QList<nap::Object*>& objects)
{
	mSelection.clear();
	mSelection.append(objects);

	for (const auto ob : objects) {
		if (ob->getTypeInfo().isKindOf<nap::PatchComponent>()) {
			setActivePatch(&static_cast<nap::PatchComponent*>(ob)->getPatch());
		}
	}

	selectionChanged(mSelection);

	actionStore().updateStates();
}

void AppContext::setActivePatch(nap::Patch* patch)
{
	mActivePatch = patch;

	// Invalidate active patch when it's deleted
	mActivePatch->removed.connect([=](const nap::Object& obj) {
		if (mActivePatch == &obj) mActivePatch = nullptr;
	});
}


void AppContext::execute(QUndoCommand* cmd)
{
	nap::Logger::debug("Command: %s", cmd->actionText().toStdString().c_str());
	mUndoStack.push(cmd);
}
QString AppContext::serialize(nap::Object* obj)
{
	assert(obj);
	std::stringstream ss;
	nap::XMLSerializer ser(ss, core());
    ser.writeObject(*obj, <#initializer#>, false);
	return QString::fromStdString(ss.str());
}
void AppContext::deserialize(const QString& data, nap::Object* parent)
{
	std::stringstream ss(data.toStdString());
	nap::XMLDeserializer ser(ss, core());
	ser.readObject(parent);
}

IconStore& AppContext::iconStore()
{
	// Late initialization, let more important things happen first
	if (!mIconStore) mIconStore = std::unique_ptr<IconStore>(new IconStore);
	return *mIconStore;
}

ActionStore& AppContext::actionStore()
{
	// Late initialization, let more important things happen first
	if (!mActionStore) mActionStore = std::unique_ptr<ActionStore>(new ActionStore);
	return *mActionStore;
}


bool AppContext::isSaved() const { return QFileInfo(mFilename).exists(); }

bool AppContext::isDirty() const { return !mUndoStack.isClean(); }

void AppContext::setClean() { mUndoStack.setClean(); }
