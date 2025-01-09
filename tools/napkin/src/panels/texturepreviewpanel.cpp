/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "texturepreviewpanel.h"
#include "../appcontext.h"

// External includes
#include <rtti/jsonwriter.h>

namespace napkin
{
	TexturePreviewPanel::TexturePreviewPanel(QWidget* parent) : StageWidget("Texture Preview",
		{ RTTI_OF(nap::Texture), RTTI_OF(nap::IMesh)}, parent)
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

		// Send event
		nap::APIEventPtr load_event = nullptr;
		if (path.getObject()->get_type().is_derived_from(RTTI_OF(nap::Texture)))
		{
			// Create load event
			bool frame = mLoadedTexture != path.getObject();
			load_event = std::make_unique<nap::APIEvent>(LoadTextureComponent::loadTextureCmd);
			load_event->addArgument<nap::APIString>(LoadTextureComponent::loadTextureArg1, writer.GetJSON());
			load_event->addArgument<nap::APIBool>(LoadTextureComponent::loadTextureArg2, frame);
			mLoadedTexture = path.getObject();
		}
		else
		{
			bool frame = mLoadedMesh != path.getObject();
			assert(path.getObject()->get_type().is_derived_from(RTTI_OF(nap::IMesh)));
			load_event = std::make_unique<nap::APIEvent>(LoadTextureComponent::loadMeshCmd);
			load_event->addArgument<nap::APIString>(LoadTextureComponent::loadMeshArg1, writer.GetJSON());
			load_event->addArgument<nap::APIBool>(LoadTextureComponent::loadMeshArg2, frame);
			mLoadedMesh = path.getObject();
		}
		mRunner.sendEvent(std::move(load_event));
	}


	void TexturePreviewPanel::clearPath()
	{
		// Send clear command to applet
		nap::APIEventPtr clear_tex_event = std::make_unique<nap::APIEvent>(LoadTextureComponent::clearCmd);
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
		auto init_future = mRunner.start(preview_app, 60, true);

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


