#include "appcontext.h"
#include "napkinglobals.h"
#include "napkinlinkresolver.h"
// std
#include <fstream>

// qt
#include <QSettings>
#include <QDir>
#include <QTimer>

// nap
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <nap/logger.h>

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
	closeDocument();
}


AppContext& AppContext::get()
{
	assert(appContextInstance != nullptr);
	return *appContextInstance;
}


void AppContext::create()
{
	assert(appContextInstance == nullptr);
    appContextInstance = std::make_unique<AppContext>();
}


void AppContext::destroy()
{
    appContextInstance = nullptr;
}


Document* AppContext::loadDocument(const QString& filename)
{
	mCurrentFilename = filename;

	nap::Logger::info("Loading '%s'", toLocalURI(filename.toStdString()).c_str());

	addRecentlyOpenedFile(filename);

	ErrorState err;
	nap::rtti::DeserializeResult result;
	std::string buffer;
	if (!readFileToString(filename.toStdString(), buffer, err))
	{
		nap::Logger::error(err.toString());
		return nullptr;
	}

	return loadDocumentFromString(buffer, filename);
}

void AppContext::reloadDocument()
{
	loadDocument(mCurrentFilename);
}

Document* AppContext::newDocument()
{
	// Create new document
	closeDocument();
	mDocument = std::make_unique<Document>(getCore());
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
	auto& factory = getCore().getResourceManager()->getFactory();

	if (!deserializeJSON(data, EPropertyValidationMode::AllowMissingProperties, factory, result, err))
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
	mDocument = std::make_unique<Document>(mCore, filename, std::move(result.mReadObjects));

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

	addRecentlyOpenedFile(filename);

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


void AppContext::openRecentDocument()
{
	auto lastFilename = AppContext::get().getLastOpenedFilename();
	if (lastFilename.isNull())
		return;
	AppContext::get().loadDocument(lastFilename);
}

const QString AppContext::getLastOpenedFilename()
{
	auto recent = getRecentlyOpenedFiles();
	if (recent.isEmpty())
		return {};

	return recent.last();
}

void AppContext::addRecentlyOpenedFile(const QString& filename)
{
	auto recentFiles = getRecentlyOpenedFiles();
	recentFiles.removeAll(filename);
	recentFiles << filename;
	while (recentFiles.size() > MAX_RECENT_FILES)
		recentFiles.removeFirst();
	QSettings().setValue(settingsKey::RECENTLY_OPENED, recentFiles);

}

QStringList AppContext::getRecentlyOpenedFiles() const
{
	return QSettings().value(settingsKey::RECENTLY_OPENED, QStringList()).value<QStringList>();
}

void AppContext::restoreUI()
{
	getThemeManager().watchThemeDir();

	// Restore theme
	QString recentTheme = QSettings().value(settingsKey::LAST_THEME, napkin::TXT_THEME_NATIVE).toString();
	if (recentTheme.isEmpty())
		recentTheme = napkin::TXT_THEME_NATIVE;

	getThemeManager().setTheme(recentTheme);

	// Let the ui come up before loading all the recent file and initializing core
	QTimer::singleShot(100, [this]() {
		openRecentDocument();
	});
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

nap::Core& AppContext::getCore()
{
	if (!mCoreInitialized)
	{
		ErrorState err;
		if (!mCore.initializeEngine(err, getExecutableDir(), true))
		{
			nap::Logger::fatal("Failed to initialize engine");
		}
		mCoreInitialized = true;
		coreInitialized();
	}
	return mCore;
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

