#include "appcontext.h"
#include "napkinglobals.h"

// std
#include <fstream>

// qt
#include <QSettings>
#include <QtCore/QDir>
#include <QtWidgets/QMessageBox>

// nap
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <rtti/defaultlinkresolver.h>

// local
#include <generic/naputils.h>
#include <nap/logger.h>
#include <utility/fileutils.h>

using namespace nap::rtti;
using namespace nap::utility;
using namespace napkin;

AppContext::AppContext()
{
	ErrorState err;
	if (!getCore().initializeEngine(err))
	{
		nap::Logger::fatal("Failed to initialize engine");
	}

	newDocument();
}

AppContext::~AppContext()
{
    mCore.shutdown();
}

AppContext& AppContext::get()
{
	static AppContext inst;
	return inst;
}

Document* AppContext::newDocument()
{
	mDocument = std::make_unique<Document>(mCore);
	connectDocumentSignals();
	newFileCreated();
	documentChanged();
	return mDocument.get();
}

Document* AppContext::loadFile(const QString& filename)
{
	auto abspath = getAbsolutePath(filename.toStdString());
	nap::Logger::info("Loading '%s'", abspath.c_str());

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

	// transfer
	mDocument = std::make_unique<Document>(mCore, filename, std::move(result.mReadObjects));
	connectDocumentSignals();
	fileOpened(filename);
	documentChanged();
	return mDocument.get();
}

void AppContext::saveFile()
{
	if (getDocument()->getCurrentFilename().isEmpty())
	{
		nap::Logger::fatal("Cannot save file, no filename has been set.");
		return;
	}
	saveFileAs(getDocument()->getCurrentFilename());
}

void AppContext::saveFileAs(const QString& filename)
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

	fileSaved(filename);
	getUndoStack().setClean();
	documentChanged();
}

void AppContext::openRecentFile()
{
	auto lastFilename = AppContext::get().getLastOpenedFilename();
	if (lastFilename.isNull())
		return;
	AppContext::get().loadFile(lastFilename);
}

const QString AppContext::getLastOpenedFilename()
{
	return QSettings().value(settingsKey::LAST_OPENED_FILE).toString();
}


void AppContext::restoreUI()
{
	// Restore theme
	const QString& recentTheme = QSettings().value(settingsKey::LAST_THEME, napkin::TXT_DEFAULT_THEME).toString();
	getThemeManager().setTheme(recentTheme);

	openRecentFile();
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
	connect(&mDocument->getUndoStack(), &QUndoStack::indexChanged, [this](int idx)
	{
		documentChanged();
	});
}

