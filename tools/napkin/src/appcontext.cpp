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
#include <renderservice.h>
#include <constantshader.h>

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
	nap::Logger::info("Loading data '%s'", toLocalURI(filename.toStdString()).c_str());
	ErrorState err; nap::rtti::DeserializeResult result; std::string buffer;

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
	if (!QFileInfo(projectFilename).exists())
	{
		nap::Logger::error("Failed to load project, unable to locate file: %s",
			projectFilename.toStdString().c_str());
		return nullptr;
	}

	// If there's a project already loaded in the current context, quit and restart.
	// The editor can only load 1 project because it needs to load modules that can't be freed.
	QString project_file_name = QString::fromStdString(nap::utility::forceSeparator(projectFilename.toStdString()));
	if (getProjectInfo() != nullptr)
	{
		QProcess proc;
		proc.setProgram(qApp->arguments()[0]);
		proc.setArguments({ "-p", project_file_name });
		proc.startDetached();
		return nullptr;
	}

	// Initialize engine
	ErrorState err;
	progressChanged(0.25f, "Loading: " + project_file_name);
	if (!mCore.initializeEngine(project_file_name.toStdString(), nap::ProjectInfo::EContext::Editor, err))
	{
		nap::Logger::error(err.toString());
		progressChanged(1.0f);
		return nullptr;
	}
	progressChanged(0.5f);

	// Clone current project information -> allows us to edit it in Napkin
	mProjectInfo = mCore.getProjectInfo()->clone();

	// Load service configuration
	mServiceConfig = std::make_unique<ServiceConfig>(mCore, *mProjectInfo);

	// Signal initialization
	coreInitialized();
	progressChanged(0.75f);

	// Enable shader compilation if render service has been loaded
	mRenderService = mCore.getService<nap::RenderService>();
	if (mRenderService != nullptr)
	{
		nap::Logger::info("Initializing %s", mRenderService->getTypeName().data());
		if (!mRenderService->initShaderCompilation(err))
		{
			nap::Logger::error(err.toString());
			progressChanged(1.0f);
			return nullptr;
		}
	}

	// Load document (data file)
	addRecentlyOpenedProject(project_file_name);
	auto dataFilename = QString::fromStdString(mCore.getProjectInfo()->getDataFile());
	if (!dataFilename.isEmpty())
		loadDocument(dataFilename);
	else
		nap::Logger::error("No data file specified");

	// All good
	projectLoaded(*mProjectInfo);
	progressChanged(1.0f);

	return mProjectInfo.get();
}

const nap::ProjectInfo* AppContext::getProjectInfo() const
{
	return mProjectInfo.get();
}


void AppContext::reloadDocument()
{
	// Get current document
	auto* doc = getDocument();
	if (doc != nullptr && !doc->getFilename().isNull())
	{
		loadDocument(doc->getFilename());
	}
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


Document* AppContext::loadDocumentFromString(const std::string& data, QString filename)
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
	ObjectList ser_objects;
	const auto& doc_objects = getDocument()->getObjects();
	for (auto& obj : doc_objects)
	{
		if (!obj.second->get_type().is_derived_from<nap::InstancePropertyValue>())
		{
			ser_objects.emplace_back(obj.second.get());
		}
	}

	JSONWriter writer; ErrorState err;
	if (!serializeObjects(ser_objects, writer, err))
	{
		nap::Logger::fatal(err.toString());
		return {};
	}
	return writer.GetJSON();
}


void AppContext::openRecentProject()
{
	auto lastFilename = AppContext::get().getLastOpenedProjectFilename();
	if (!lastFilename.isNull())
	{
		AppContext::get().loadProject(lastFilename);
	}
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
	QString file_name = QString::fromStdString(nap::utility::forceSeparator(filename.toStdString()));
	auto recentProjects = getRecentlyOpenedProjects();
	recentProjects.removeAll(file_name);
	recentProjects << file_name;
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
		connect(doc, &Document::childEntityAdded, this, &AppContext::childEntityAdded);
		connect(doc, &Document::objectAdded, this, &AppContext::objectAdded);
		connect(doc, &Document::removingObject, this, &AppContext::removingObject);
		connect(doc, &Document::objectRemoved, this, &AppContext::objectRemoved);
		connect(doc, &Document::objectReparenting, this, &AppContext::objectReparenting);
		connect(doc, &Document::objectReparented, this, &AppContext::objectReparented);
		connect(doc, &Document::objectRenamed, this, &AppContext::objectRenamed);
		connect(doc, &Document::propertyValueChanged, this, &AppContext::propertyValueChanged);
		connect(doc, &Document::propertyChildInserted, this, &AppContext::propertyChildInserted);
		connect(doc, &Document::propertyChildRemoved, this, &AppContext::propertyChildRemoved);
		connect(doc, &Document::arrayIndexSwapped, this, &AppContext::arrayIndexSwapped);
		connect(&mDocument->getUndoStack(), &QUndoStack::indexChanged, this, &AppContext::onUndoIndexChanged);
	}
	else
	{
		disconnect(doc, &Document::childEntityAdded, this, &AppContext::childEntityAdded);
		disconnect(doc, &Document::objectAdded, this, &AppContext::objectAdded);
		disconnect(doc, &Document::removingObject, this, &AppContext::removingObject);
		disconnect(doc, &Document::objectRemoved, this, &AppContext::objectRemoved);
		disconnect(doc, &Document::objectReparenting, this, &AppContext::objectReparenting);
		disconnect(doc, &Document::objectReparented, this, &AppContext::objectReparented);
		disconnect(doc, &Document::objectRenamed, this, &AppContext::objectRenamed);
		disconnect(doc, &Document::propertyValueChanged, this, &AppContext::propertyValueChanged);
		disconnect(doc, &Document::propertyChildInserted, this, &AppContext::propertyChildInserted);
		disconnect(doc, &Document::propertyChildRemoved, this, &AppContext::propertyChildRemoved);
		disconnect(doc, &Document::arrayIndexSwapped, this, &AppContext::arrayIndexSwapped);
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
	return mDocument == nullptr ? newDocument() : mDocument.get();
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


bool napkin::AppContext::canRender() const
{
	return mRenderService != nullptr;
}


napkin::ServiceConfig* napkin::AppContext::getServiceConfig() const
{
	return mServiceConfig.get();
}


nap::RenderService* napkin::AppContext::getRenderService() const
{
	return mRenderService;
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
	if (!hasDocument())
	{
		nap::Logger::warn("Unable to execute command '%s' - Document not loaded",
			cmd->text().toStdString().c_str());
		return;
	}
	getDocument()->executeCommand(cmd);
}


bool napkin::AppContext::getProjectLoaded() const
{
	return mProjectInfo != nullptr;
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

