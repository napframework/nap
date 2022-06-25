/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "appcontext.h"
#include "napkinglobals.h"
#include "napkinlinkresolver.h"

// std
#include <fstream>

// qt
#include <QSettings>
#include <QTimer>
#include <QProcess>
#include <QDir>

// nap
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <mathutils.h>

// local
#include <naputils.h>
#include <utility/fileutils.h>

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
	// Close data file
	closeDocument();

	// Close service configuration
	closeServiceConfiguration();

	// Clear project info
	mProjectInfo.reset(nullptr);
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
	mCurrentFilename = filename;
	nap::Logger::info("Loading data '%s'", toLocalURI(filename.toStdString()).c_str());

	ErrorState err;
	nap::rtti::DeserializeResult result;
	std::string buffer;

	if (!QFile::exists(filename))
	{
		nap::Logger::error("File not found: %s", filename.toStdString().c_str());
		return nullptr;
	}

	if (!QFileInfo(filename).isFile())
	{
		nap::Logger::error("Not a file: %s", filename.toStdString().c_str());
		return nullptr;
	}

	if (!readFileToString(filename.toStdString(), buffer, err))
	{
		nap::Logger::error(err.toString());
		return nullptr;
	}
	return loadDocumentFromString(buffer, filename);
}

const nap::ProjectInfo* AppContext::loadProject(const QString& projectFilename)
{
	// If there's a project already loaded in the current context, quit and restart.
	// The editor can only load 1 project because it needs to load modules that can't be freed.
	progressChanged(0.25f, "Loading: " + projectFilename);
	if (getProjectInfo() != nullptr)
	{
		QProcess::startDetached(qApp->arguments()[0], {"-p", projectFilename});
		getQApplication()->exit(0);
		return nullptr;
	}

	// Initialize engine
	ErrorState err;
	if (!mCore.initializeEngine(projectFilename.toStdString(), nap::ProjectInfo::EContext::Editor, err))
	{
		nap::Logger::error(err.toString());
		progressChanged(1.0f);
		return nullptr;
	}
	progressChanged(0.5f);

	// Clone current project information, allows us to edit it
	const auto* project_info = mCore.getProjectInfo();
	mProjectInfo = nap::rtti::cloneObject(*project_info, mCore.getResourceManager()->getFactory());
	mProjectInfo->setFilename(project_info->getFilename());

	// Load service configuration
	mServiceConfig = std::make_unique<ServiceConfig>(mCore, *mProjectInfo);

	// Signal initialization
	coreInitialized();
	progressChanged(0.75f);

	// Load document (data file)
	addRecentlyOpenedProject(projectFilename);
	auto dataFilename = QString::fromStdString(mCore.getProjectInfo()->getDataFile());
	if (!dataFilename.isEmpty())
		loadDocument(dataFilename);
	else
		nap::Logger::error("No data file specified");

	// All good
	progressChanged(1.0f);
	return mCore.getProjectInfo();
}


const nap::ProjectInfo* AppContext::getProjectInfo() const
{
	return mProjectInfo.get();
}


void AppContext::reloadDocument()
{
	loadDocument(mCurrentFilename);
}

Document* AppContext::newDocument()
{
	// Close current document if available
	closeDocument();

	// No instance of core provided
	nap::Core& core = getCore();
	if (!core.isInitialized())
	{
		nap::Logger::warn("NAP not initialized, cannot create document");
		return nullptr;
	}

	// Create document
	mDocument = std::make_unique<Document>(core);
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
	nap::Core& core = getCore();
	if (!core.isInitialized())
	{
		nap::Logger::warn("Core not initialized, cannot load document");
		return nullptr;
	}

	auto& factory = core.getResourceManager()->getFactory();
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
	mDocument = std::make_unique<Document>(core, filename, std::move(result.mReadObjects));

	// Notify listeners
	connectDocumentSignals();
	documentOpened(filename);
	Document* doc = mDocument.get();
	documentChanged(doc);
	return doc;
}


bool AppContext::saveDocument()
{
	if (getDocument()->getFilename().isEmpty())
	{
		nap::Logger::fatal("Cannot save file, no filename has been set.");
		return false;
	}
	return saveDocumentAs(getDocument()->getFilename());
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
	nap::Logger::info("Written '%s'", toLocalURI(filename.toStdString()).c_str());

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
	if (getProjectInfo() == nullptr && mOpenRecentProjectAtStartup)
	{
		openRecentProject();
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
		connect(doc, &Document::objectReparenting, this, &AppContext::objectReparenting);
		connect(doc, &Document::objectReparented, this, &AppContext::objectReparented);
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
		disconnect(doc, &Document::objectReparenting, this, &AppContext::objectReparenting);
		disconnect(doc, &Document::objectReparented, this, &AppContext::objectReparented);
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
		auto mainWin = qobject_cast<QMainWindow*>(window);
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

nap::Core& AppContext::getCore()
{
	return mCore;
}

Document* AppContext::getDocument()
{
	if (mDocument == nullptr)
		newDocument();
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


nap::ProjectInfo* napkin::AppContext::getProjectInfo()
{
	return mProjectInfo.get();
}


bool napkin::AppContext::documentIsProjectDefault() const
{
	assert(hasDocument());
	QDir proj_dir(QString::fromStdString(AppContext::get().getProjectInfo()->getProjectDir()));
	auto cur_file = proj_dir.relativeFilePath(mDocument->getFilename()).toStdString();
	return cur_file == getProjectInfo()->mDefaultData;
}


bool napkin::AppContext::hasDocument() const
{
	return mDocument != nullptr;
}


bool napkin::AppContext::hasServiceConfig() const
{
	return mServiceConfig != nullptr;
}


const napkin::ServiceConfig* napkin::AppContext::getServiceConfig() const
{
	return mServiceConfig.get();
}


QApplication* napkin::AppContext::getQApplication() const
{
	return qobject_cast<QApplication*>(qGuiApp);
}


QUndoStack& napkin::AppContext::getUndoStack()
{
	return getDocument()->getUndoStack();
}


napkin::ThemeManager& napkin::AppContext::getThemeManager()
{
	return mThemeManager;
}


void napkin::AppContext::executeCommand(QUndoCommand* cmd)
{
	getDocument()->executeCommand(cmd);
}


napkin::ServiceConfig* napkin::AppContext::getServiceConfig()
{
	return mServiceConfig.get();
}


void napkin::AppContext::closeDocument()
{
	if (mDocument == nullptr)
		return;

	documentClosing(mDocument->getFilename());
	mDocument.reset(nullptr);
}


void napkin::AppContext::closeServiceConfiguration()
{
	// Close configuration
	if (mServiceConfig != nullptr)
	{
		serviceConfigurationClosing(mServiceConfig->getDocument().getFilename());
		mServiceConfig.reset(nullptr);
	}
}


void AppContext::setOpenRecentProjectOnStartup(bool b)
{
	mOpenRecentProjectAtStartup = b;
}

