#include "appcontext.h"

// std
#include <fstream>

// qt
#include <QSettings>
#include <QTimer>
#include <QProcess>

// nap
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>

// local
#include <naputils.h>
#include <utility/fileutils.h>
#include "napkinglobals.h"
#include "napkinlinkresolver.h"

using namespace nap::rtti;
using namespace nap::utility;
using namespace napkin;

std::unique_ptr<AppContext> appContextInstance = nullptr;


AppContext::AppContext()
{
	nap::Logger::instance().log.connect(mLogHandler);
}


AppContext::~AppContext()
{
	closeDocument();
}


AppContext& AppContext::get()
{
	assert(appContextInstance != nullptr);
	return *appContextInstance;
}


AppContext& AppContext::create()
{
	assert(appContextInstance == nullptr);
    appContextInstance = std::make_unique<AppContext>();
    return *appContextInstance;
}


void AppContext::destroy()
{
    appContextInstance = nullptr;
}


Document* AppContext::loadDocument(const QString& filename)
{
	blockingProgressChanged(0, "Loading: " + filename);

	mCurrentFilename = filename;

	nap::Logger::info("Loading '%s'", toLocalURI(filename.toStdString()).c_str());

	ErrorState err;
	nap::rtti::DeserializeResult result;
	std::string buffer;

	if (!QFile::exists(filename))
	{
		blockingProgressChanged(1);

		nap::Logger::error("File not found: %s", filename.toStdString().c_str());
		return nullptr;
	}

	if (!QFileInfo(filename).isFile())
	{
		blockingProgressChanged(1);

		nap::Logger::error("Not a file: %s", filename.toStdString().c_str());
		return nullptr;
	}

	if (!readFileToString(filename.toStdString(), buffer, err))
	{
		blockingProgressChanged(1);

		nap::Logger::error(err.toString());
		return nullptr;
	}

	blockingProgressChanged(1);

	return loadDocumentFromString(buffer, filename);
}

nap::ProjectInfo* AppContext::loadProject(const QString& projectFilename)
{
	// TODO: See if we can run this on a thread so the progress dialog may update live.

	blockingProgressChanged(0, "Loading: " + projectFilename);

	// Workaround for issues while unloading the current project: just start a new instance.
	if (getProject())
	{
		QProcess::startDetached(qApp->arguments()[0], {"-p", projectFilename});
		getQApplication()->exit(0);
		return nullptr;
	}

	ErrorState err;

	mCore = std::make_unique<nap::Core>();

	if (!mCore->loadProjectInfo(err, projectFilename.toStdString()))
	{
		blockingProgressChanged(1);
		nap::Logger::error("Failed to load project info %s: %s",
						   projectFilename.toStdString().c_str(), err.toString().c_str());

		if (mExitOnLoadFailure)
			exit(1);
		return nullptr;

	}

	mCore->getProjectInfo()->setEditorMode(true);

//	if (err.hasErrors())
//	{
//		blockingProgressChanged(1);
//		nap::Logger::error("Failed to load project info %s: %s",
//						   projectFilename.toStdString().c_str(), err.toString().c_str());
//
//		if (mExitOnLoadFailure)
//			exit(1);
//
//		return nullptr;
//	}
//
//	projectInfo->getFilename() = projectFilename.toStdString();
//	if (!mCore->loadPathMapping(*projectInfo, err))
//	{
//		blockingProgressChanged(1);
//
//		nap::Logger::error("Failed to load path mapping %s: %s",
//						   projectInfo->mPathMappingFile.c_str(), err.toString().c_str());
//
//		if (mExitOnLoadFailure)
//			exit(1);
//
//		return nullptr;
//	}

//	nap::Logger::info("Loading project '%s' ver. %s (%s)",
//					  projectInfo->mTitle.c_str(),
//					  projectInfo->mVersion.c_str(),
//					  projectInfo->getProjectDir().c_str());

	if (!mCore->doInitializeEngine(err))
	{
		blockingProgressChanged(1);

		nap::Logger::error(err.toString());

		if (mExitOnLoadFailure) 
			exit(1);

		return nullptr;
	}

	coreInitialized();

    if (mExitOnLoadSuccess) {
		nap::Logger::info("Loaded successfully, exiting as requested");
        exit(EXIT_ON_SUCCESS_EXIT_CODE);
    }

	addRecentlyOpenedProject(projectFilename);

	auto dataFilename = QString::fromStdString(mCore->getProjectInfo()->getDefaultDataFile());
	if (!dataFilename.isEmpty())
		loadDocument(dataFilename);
	else
	{
		blockingProgressChanged(1);

		nap::Logger::warn("No data file specified");
		if (mExitOnLoadFailure) 
			exit(1);
	}

	blockingProgressChanged(1);

	return mCore->getProjectInfo();
}

nap::ProjectInfo* AppContext::getProject() const
{
	if (!mCore)
		return nullptr;

	return mCore->getProjectInfo();
}

void AppContext::reloadDocument()
{
	loadDocument(mCurrentFilename);
}

Document* AppContext::newDocument()
{
	// Create new document
	closeDocument();
	auto core = getCore();
	if (!core)
	{
		nap::Logger::warn("Core not loaded, cannot create document");
		return nullptr;
	}
	mDocument = std::make_unique<Document>(*core);
	connectDocumentSignals();

	// Notify listeners
	newDocumentCreated();
	documentChanged(mDocument.get());
	return mDocument.get();
}

Document* AppContext::loadDocumentFromString(const std::string& data, const QString& filename)
{
	ErrorState err;
	nap::rtti::DeserializeResult result;
	auto core = getCore();
	if (!core)
	{
		nap::Logger::warn("Core not loaded, cannot load document");
		return nullptr;
	}

	auto& factory = core->getResourceManager()->getFactory();

	if (!deserializeJSON(data, EPropertyValidationMode::AllowMissingProperties, EPointerPropertyMode::NoRawPointers, factory, result, err))
	{
		nap::Logger::error(err.toString());
		return nullptr;
	}

	if (!NapkinLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, err))
	{
		nap::Logger::error("Failed to resolve links: %s", err.toString().c_str());
		return nullptr;
	}

	// Create new document
	closeDocument();

	mDocument = std::make_unique<Document>(*core, filename, std::move(result.mReadObjects));

	// Notify listeners
	connectDocumentSignals();
	documentOpened(filename);
	Document* doc = mDocument.get();
	documentChanged(doc);
	return doc;
}


bool AppContext::saveDocument()
{
	if (getDocument()->getCurrentFilename().isEmpty())
	{
		nap::Logger::fatal("Cannot save file, no filename has been set.");
		return false;
	}
	return saveDocumentAs(getDocument()->getCurrentFilename());
}

bool AppContext::saveDocumentAs(const QString& filename)
{

	std::string serialized_document = documentToString();
	if (serialized_document.empty())
		return false;

	std::ofstream out(filename.toStdString());
	out << serialized_document;
	out.close();

	getDocument()->setFilename(filename);

	nap::Logger::info("Written file: " + filename.toStdString());

	documentSaved(filename);
	getUndoStack().setClean();
	documentChanged(mDocument.get());

	return true;
}

std::string AppContext::documentToString() const
{
	ObjectList objects;
	for (auto& ob : getDocument()->getObjects())
	{
		if (ob->get_type().is_derived_from<nap::InstancePropertyValue>())
			continue;
		objects.emplace_back(ob.get());
	}

	JSONWriter writer;
	ErrorState err;
	if (!serializeObjects(objects, writer, err))
	{
		nap::Logger::fatal(err.toString());
		return {};
	}
	return writer.GetJSON();
}


void AppContext::openRecentProject()
{
	auto lastFilename = AppContext::get().getLastOpenedProjectFilename();
	if (lastFilename.isNull())
		return;
	AppContext::get().loadProject(lastFilename);
}

const QString AppContext::getLastOpenedProjectFilename()
{
	auto recent = getRecentlyOpenedProjects();
	if (recent.isEmpty())
		return {};

	return recent.last();
}

void AppContext::addRecentlyOpenedProject(const QString& filename)
{
	auto recentProjects = getRecentlyOpenedProjects();
	recentProjects.removeAll(filename);
	recentProjects << filename;
	while (recentProjects.size() > MAX_RECENT_FILES)
		recentProjects.removeFirst();
	QSettings().setValue(settingsKey::RECENTLY_OPENED, recentProjects);

}

QStringList AppContext::getRecentlyOpenedProjects() const
{
	return QSettings().value(settingsKey::RECENTLY_OPENED, QStringList()).value<QStringList>();
}

void AppContext::restoreUI()
{
	getThemeManager().watchThemeDir();

	// Restore theme
	QString recentTheme = QSettings().value(settingsKey::LAST_THEME, napkin::TXT_THEME_DEFAULT).toString();
	getThemeManager().setTheme(recentTheme);

	// Let the ui come up before loading all the recent file and initializing core
	if (!getProject() && mOpenRecentProjectAtStartup)
	{
		QTimer::singleShot(100, [this]()
		{
			openRecentProject();
		});
	}
}

void AppContext::connectDocumentSignals(bool enable)
{
    
	auto doc = mDocument.get();
	if (!doc) 
        return;

    if (enable)
	{
		connect(doc, &Document::entityAdded, this, &AppContext::entityAdded);
		connect(doc, &Document::componentAdded, this, &AppContext::componentAdded);
		connect(doc, &Document::objectAdded, this, &AppContext::objectAdded);
		connect(doc, &Document::objectChanged, this, &AppContext::objectChanged);
		connect(doc, &Document::objectRemoved, this, &AppContext::objectRemoved);
		connect(doc, &Document::propertyValueChanged, this, &AppContext::propertyValueChanged);
		connect(doc, &Document::propertyChildInserted, this, &AppContext::propertyChildInserted);
		connect(doc, &Document::propertyChildRemoved, this, &AppContext::propertyChildRemoved);
		connect(&mDocument->getUndoStack(), &QUndoStack::indexChanged, this, &AppContext::onUndoIndexChanged);
	}
	else
	{
		disconnect(doc, &Document::entityAdded, this, &AppContext::entityAdded);
		disconnect(doc, &Document::componentAdded, this, &AppContext::componentAdded);
		disconnect(doc, &Document::objectAdded, this, &AppContext::objectAdded);
		disconnect(doc, &Document::objectChanged, this, &AppContext::objectChanged);
		disconnect(doc, &Document::objectRemoved, this, &AppContext::objectRemoved);
		disconnect(doc, &Document::propertyValueChanged, this, &AppContext::propertyValueChanged);
		disconnect(doc, &Document::propertyChildInserted, this, &AppContext::propertyChildInserted);
		disconnect(doc, &Document::propertyChildRemoved, this, &AppContext::propertyChildRemoved);
		disconnect(&mDocument->getUndoStack(), &QUndoStack::indexChanged, this, &AppContext::onUndoIndexChanged);
	}
}

QMainWindow* AppContext::getMainWindow() const
{
	for (auto window : getQApplication()->topLevelWidgets())
	{
		auto mainWin = dynamic_cast<QMainWindow*>(window);
		if (mainWin != nullptr)
			return mainWin;
	}
	return nullptr;
}


void AppContext::handleURI(const QString& uri)
{
	// Match object
	QRegularExpression objectLink(QString("%1:\\/\\/([^@]+)").arg(QString::fromStdString(NAP_URI_PREFIX)));
	auto match = objectLink.match(uri);
	if (match.hasMatch())
	{
		auto objname = match.captured(1);
		auto obj = getDocument()->getObject(objname.toStdString());
		if (obj == nullptr)
			return;
		selectionChanged({obj});

		// Match property
		QRegularExpression proplink(QString("%1:\\/\\/([^@]+)@(.+)").arg(QString::fromStdString(NAP_URI_PREFIX)));
		match = proplink.match(uri);
		if (match.hasMatch())
		{
			auto proppath = match.captured(2);
			PropertyPath path(*obj, nap::rtti::Path::fromString(proppath.toStdString()), *mDocument);
			propertySelectionChanged(path);
		}
		return;
	}

	// File path
	QRegularExpression filelink("file:\\/\\/(.+)");
	match = filelink.match(uri);
	if (match.hasMatch()) {
		nap::qt::revealInFileBrowser(QString::fromStdString(fromLocalURI(uri.toStdString())));
	}
}

nap::Core* AppContext::getCore()
{
//	if (!mCore)
//	{
//		ErrorState err;
//		if (!mCore->initializeEngine(err, getExecutableDir(), true))
//		{
//			nap::Logger::fatal("Failed to initialize engine");
//			return nullptr;
//		}
//		coreInitialized();
//	}
	return mCore.get();
}

Document* AppContext::getDocument()
{
	if (mDocument == nullptr)
	{
		newDocument();
	}
	return mDocument.get();
}

const Document* AppContext::getDocument() const
{
	return mDocument.get();
}

void AppContext::onUndoIndexChanged()
{
	documentChanged(mDocument.get());
}


bool napkin::AppContext::isAvailable()
{
	return appContextInstance != nullptr;
}


void napkin::AppContext::closeDocument()
{
	if (mDocument == nullptr)
		return;

	QString prev_doc_name = mDocument->getCurrentFilename();
	documentClosing(prev_doc_name);
	mDocument.reset(nullptr);
}

void AppContext::setOpenRecentProjectOnStartup(bool b)
{
	mOpenRecentProjectAtStartup = b;
}

void AppContext::setExitOnLoadFailure(bool b)
{
	mExitOnLoadFailure = b;
}

void AppContext::setExitOnLoadSuccess(bool b)
{
	mExitOnLoadSuccess = b;
}
