/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "meshpreviewpanel.h"
#include "../appcontext.h"
#include "../applets/meshpreviewapicomponent.h"

// External includes
#include <apiservice.h>
#include <rtti/jsonwriter.h>
#include <apievent.h>
#include <trianglemesh.h>

namespace napkin
{
	static constexpr const char* sPanelName = "Mesh Preview";

	MeshPreviewPanel::MeshPreviewPanel(QWidget * parent) : StageWidget(sPanelName,
		{ RTTI_OF(nap::IMesh) }, { RTTI_OF(TriangleMesh) }, RTTI_OF(nap::IMesh), parent)
	{
		// Create render resources on project load
		connect(&AppContext::get(), &AppContext::projectLoaded, this, &MeshPreviewPanel::init);
	}


	MeshPreviewPanel::~MeshPreviewPanel()
	{	
		mRunner.abort();
	}


	void MeshPreviewPanel::closeEvent(QCloseEvent* event)
	{
		mRunner.abort();
		QWidget::closeEvent(event);
	}


	void MeshPreviewPanel::init(const nap::ProjectInfo& info)
	{
		// Don't do anything if  meshes aren't supported
		if (!isSupported(info))
			return;

		// Signals completion setup resources gui thread
		assert(mPanel == nullptr);

		// Create the window
		nap::utility::ErrorState error;
		mPanel = RenderPanel::create(mRunner, *this, error);
		if (!error.check(mPanel != nullptr, "Unable to create '%s' render panel", sPanelName))
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Initialize and run the applet (core, services & application)
		auto preview_app = nap::utility::forceSeparator(nap::utility::getExecutableDir() + app);
		assert(!mInitFuture.valid());
		mInitFuture = mRunner.start(preview_app, info, true);

		// Let the applet initialize on it's own thread -> install next frame
		QTimer::singleShot(0, [this]()
			{
				// Wait until initialized and bail on failure
				assert(mInitFuture.valid());
				if (!mInitFuture.get())
				{
					nap::Logger::error("'%s' initialization failed, check the log for more details", sPanelName);
					return;
				}

				// Sync theme
				const auto* theme = AppContext::get().getThemeManager().getCurrentTheme();
				assert(theme != nullptr);
				themeChanged(*theme);

				// Install layout
				assert(layout() == nullptr);
				mMasterLayout.setContentsMargins(0, 0, 0, 0);
				mMasterLayout.addWidget(&mPanel->getWidget());
				setLayout(&mMasterLayout);

				// Listen to property changes
				connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &MeshPreviewPanel::propertyValueChanged);
				connect(&AppContext::get(), &AppContext::objectRemoved, this, &MeshPreviewPanel::objectRemoved);
				connect(&AppContext::get(), &AppContext::documentClosing, this, &MeshPreviewPanel::documentClosing);
				connect(&AppContext::get().getThemeManager(), &ThemeManager::themeChanged, this, &MeshPreviewPanel::themeChanged);
			});
	}


	bool MeshPreviewPanel::onLoadPath(const PropertyPath& path, nap::utility::ErrorState& error)
	{
		// Serialize to JSON
		auto* object = path.getObject();
		assert(object != nullptr);
		assert(object->get_type().is_derived_from(RTTI_OF(IMesh)));

		nap::rtti::JSONWriter writer;
		nap::rtti::ObjectList list = { path.getObject() };
		if (!serializeObjects(list, writer, error))
			return false;

		// Send load command
		APIEventPtr load_mesh_event = std::make_unique<nap::APIEvent>(MeshPreviewAPIcomponent::loadMeshCmd);
		load_mesh_event->addArgument<nap::APIString>("data", writer.GetJSON());
		load_mesh_event->addArgument<nap::APIBool>("frame", path.getObject() != mMesh);
		mRunner.sendEvent(std::move(load_mesh_event));

		// Store for future reference
		mMesh = path.getObject();
		return true;
	}


	void MeshPreviewPanel::clear()
	{
		// Send clear command to applet
		nap::APIEventPtr clear_tex_event = std::make_unique<nap::APIEvent>(MeshPreviewAPIcomponent::clearMeshCmd);
		mRunner.sendEvent(std::move(clear_tex_event));
		mMesh = nullptr;
	}


	void MeshPreviewPanel::propertyValueChanged(const PropertyPath& path)
	{
		assert(path.isValid());
		if (path.getObject() == mMesh)
		{
			utility::ErrorState error;
			if (!onLoadPath(path, error))
				nap::Logger::error(error.toString());
		}
	}


	void MeshPreviewPanel::objectRemoved(nap::rtti::Object* object)
	{
		if (object == mMesh)
			clear();
	}


	void MeshPreviewPanel::documentClosing(const QString& doc)
	{
		clear();
	}


	void MeshPreviewPanel::themeChanged(const Theme& theme)
	{
		APIEventPtr change_theme_cmd = std::make_unique<nap::APIEvent>(MeshPreviewAPIcomponent::changeThemeCmd);
		change_theme_cmd->addArgument<nap::APIString>(MeshPreviewAPIcomponent::changeThemeArg1, theme.getName().toStdString());
		mRunner.sendEvent(std::move(change_theme_cmd));
	}
}
