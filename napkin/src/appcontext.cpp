#include "appcontext.h"
#include "napkinglobals.h"

// std
#include <fstream>

// qt
#include <QSettings>
#include <QDir>
#include <QTimer>

// nap
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <rtti/defaultlinkresolver.h>
#include <nap/logger.h>

// local
#include <generic/naputils.h>
#include <utility/fileutils.h>

using namespace nap::rtti;
using namespace nap::utility;
using namespace napkin;

AppContext::AppContext()
{
	nap::Logger::instance().log.connect(mLogHandler);
}

AppContext::~AppContext()
{}

AppContext& AppContext::get()
{
	static AppContext inst;
	return inst;
}

Document* AppContext::newDocument()
{
	mDocument = std::make_unique<Document>(getCore());
	connectDocumentSignals();
	newDocumentCreated();
	Document* doc = mDocument.get();
	documentChanged(doc);
	return doc;
}

Document* AppContext::loadDocument(const QString& filename)
{
	nap::Logger::info("Loading '%s'", toLocalURI(filename.toStdString()).c_str());

	QSettings().setValue(settingsKey::LAST_OPENED_FILE, filename);

	ErrorState err;

	nap::rtti::RTTIDeserializeResult result;
	if (!readJSONFile(filename.toStdString(), getCore().getResourceManager()->getFactory(), result, err))
	{
		nap::Logger::fatal(err.toString());
		return nullptr;
	}

	if (!nap::rtti::DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, err))
	{
		nap::Logger::fatal("Failed to resolve links: %s", err.toString().c_str());
		return nullptr;
	}

	mDocument = std::make_unique<Document>(mCore, filename, std::move(result.mReadObjects));
	connectDocumentSignals();
	documentOpened(filename);
	Document* doc = mDocument.get();
	documentChanged(doc); // Stack corruption?
	return doc;
}

void AppContext::saveDocument()
{
	if (getDocument()->getCurrentFilename().isEmpty())
	{
		nap::Logger::fatal("Cannot save file, no filename has been set.");
		return;
	}
	saveDocumentAs(getDocument()->getCurrentFilename());
}

void AppContext::saveDocumentAs(const QString& filename)
{
	ObjectList objects;
	for (auto& ob : getDocument()->getObjects())
	{
		objects.emplace_back(ob.get());
	}

	JSONWriter writer;
	ErrorState err;
	if (!serializeObjects(objects, writer, err))
	{
		nap::Logger::fatal(err.toString());
		return;
	}

	std::ofstream out(filename.toStdString());
	out << writer.GetJSON();
	out.close();

	getDocument()->setFilename(filename);

	nap::Logger::info("Written file: " + filename.toStdString());

	QSettings().setValue(settingsKey::LAST_OPENED_FILE, filename);

	documentSaved(filename);
	getUndoStack().setClean();
	documentChanged(mDocument.get());
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
	return QSettings().value(settingsKey::LAST_OPENED_FILE).toString();
}


void AppContext::restoreUI()
{
	getThemeManager().watchThemeDir();

	// Restore theme
	const QString& recentTheme = QSettings().value(settingsKey::LAST_THEME, napkin::TXT_THEME_NATIVE).toString();
	getThemeManager().setTheme(recentTheme);

	// Let the ui come up before loading all the recent file and initializing core
	QTimer::singleShot(100, [this]() {
		openRecentDocument();
	});
}

void AppContext::connectDocumentSignals()
{
	auto doc = getDocument();

	connect(doc, &Document::entityAdded, this, &AppContext::entityAdded);
	connect(doc, &Document::componentAdded, this, &AppContext::componentAdded);
	connect(doc, &Document::objectAdded, this, &AppContext::objectAdded);
	connect(doc, &Document::objectChanged, this, &AppContext::objectChanged);
	connect(doc, &Document::objectRemoved, this, &AppContext::objectRemoved);
	connect(doc, &Document::propertyValueChanged, this, &AppContext::propertyValueChanged);
	connect(&mDocument->getUndoStack(), &QUndoStack::indexChanged, this, &AppContext::onUndoIndexChanged);
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
			PropertyPath path(*obj, proppath.toStdString());
			propertySelectionChanged(path);
		}
		return;
	}

	// File path
	QRegularExpression filelink("file:\\/\\/(.+)");
	match = filelink.match(uri);
	if (match.hasMatch()) {
		revealInFileBrowser(QString::fromStdString(fromLocalURI(uri.toStdString())));
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

void AppContext::onUndoIndexChanged()
{
	documentChanged(mDocument.get());
}


