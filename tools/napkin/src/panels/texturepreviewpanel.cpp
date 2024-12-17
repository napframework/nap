/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "texturepreviewpanel.h"
#include "../appcontext.h"

#include <rtti/jsonwriter.h>

namespace napkin
{
	TexturePreviewPanel::TexturePreviewPanel(QWidget* parent) : StageWidget("Texture Preview",
		{ RTTI_OF(nap::Texture) }, parent)
	{
		// Create render resources on project load
		connect(&AppContext::get(), &AppContext::projectLoaded, this, &TexturePreviewPanel::init);
	}


	TexturePreviewPanel::~TexturePreviewPanel()
	{	
		mRunner.abort();
	}


	void TexturePreviewPanel::closeEvent(QCloseEvent* event)
	{
		mRunner.abort();
		return QWidget::closeEvent(event);
	}


	void TexturePreviewPanel::loadPath(const PropertyPath& path)
	{
		// Serialize to JSON
		nap::rtti::JSONWriter writer;
		nap::rtti::ObjectList list = { path.getObject() };
		nap::utility::ErrorState error;
		if (!serializeObjects(list, writer, error))
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Send as command to applet
		nap::APIEventPtr load_tex_event = std::make_unique<nap::APIEvent>(TexturePreviewApplet::loadCmd);
		load_tex_event->addArgument<nap::APIString>(TexturePreviewApplet::loadArg1, writer.GetJSON());
		load_tex_event->addArgument<nap::APIBool>(TexturePreviewApplet::loadArg2,
			mLoadedObject != path.getObject());
		mRunner.sendEvent(std::move(load_tex_event));

		// Cache so we know if we need to re-frame if it's new
		mLoadedObject = path.getObject();
	}


	void TexturePreviewPanel::clearPath()
	{
		// Send clear command to applet
		nap::APIEventPtr clear_tex_event = std::make_unique<nap::APIEvent>(TexturePreviewApplet::clearCmd);
		mRunner.sendEvent(std::move(clear_tex_event));
	}


	void TexturePreviewPanel::init(const nap::ProjectInfo& info)
	{
		// Signals completion setup resources gui thread
		assert(mPanel == nullptr);

		// Create the window
		nap::utility::ErrorState error;
		mPanel = RenderPanel::create(mRunner, this, error);
		if (mPanel == nullptr)
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Initializing the applet (core, services & application)
		auto preview_app = nap::utility::forceSeparator(nap::utility::getExecutableDir() + app);
		auto init_future = mRunner.start(preview_app, 60);

		// Don't install layout if initialization fails
		if (!init_future.get())
			return;

		// Install window into this widget
		assert(layout() == nullptr);
		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(&mPanel->getWidget());
		setLayout(&mLayout);
	}
}


