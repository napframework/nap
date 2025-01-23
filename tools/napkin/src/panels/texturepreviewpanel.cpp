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


	void TexturePreviewPanel::init(const nap::ProjectInfo& info)
	{
		// Should only be initialized once, after the project is loaded
		assert(mPanel == nullptr);

		// Create the nap compatible render window
		nap::utility::ErrorState error;
		mPanel = RenderPanel::create(mRunner, this, error);
		if (mPanel == nullptr)
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Initializing the applet (core, services & application)
		auto preview_app = nap::utility::forceSeparator(nap::utility::getExecutableDir() + app);
		auto init_future = mRunner.start(preview_app, true);

		// Listen to property changes
		connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &TexturePreviewPanel::propertyValueChanged);
		connect(&AppContext::get(), &AppContext::objectRemoved, this, &TexturePreviewPanel::objectRemoved);
		connect(&AppContext::get(), &AppContext::documentClosing, this, &TexturePreviewPanel::documentClosing);

		// Don't install layout if initialization fails
		if (!init_future.get())
			return;

		// Install window into this widget
		assert(layout() == nullptr);
		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(&mPanel->getWidget());
		setLayout(&mLayout);
	}


	void TexturePreviewPanel::closeEvent(QCloseEvent* event)
	{
		mRunner.abort();
		return QWidget::closeEvent(event);
	}


	bool TexturePreviewPanel::onLoadPath(const PropertyPath& path, utility::ErrorState& error)
	{
		// Find load function
		auto load = findLoader(path);
		if (!error.check(load != nullptr,
			"Unable to find loader for '%s'", path.getObject()->mID.c_str()))
			return false;

		// Attempt to load
		return load(path, true, error);
	}


	bool TexturePreviewPanel::loadTexture(const PropertyPath& path, bool frame, nap::utility::ErrorState& error)
	{
		// Serialize to JSON
		nap::rtti::JSONWriter writer;
		nap::rtti::ObjectList list = { path.getObject() };
		if (!serializeObjects(list, writer, error))
			return false;

		// Create load event
		APIEventPtr load_tex_event = std::make_unique<nap::APIEvent>(LoadTextureComponent::loadTextureCmd);
		load_tex_event->addArgument<nap::APIString>(LoadTextureComponent::loadTextureArg1, writer.GetJSON());
		load_tex_event->addArgument<nap::APIBool>(LoadTextureComponent::loadTextureArg2, frame);
		mRunner.sendEvent(std::move(load_tex_event));

		// Store for property changes
		mLoadedTexture = rtti_cast<Texture>(path.getObject());
		assert(mLoadedTexture != nullptr);
		mTrackedObject = mLoadedTexture;
		return true;
	}


	bool TexturePreviewPanel::loadMesh(const PropertyPath& path, bool frame, nap::utility::ErrorState& error)
	{
		// Bail if no texture is loaded
		if (!error.check(mLoadedTexture != nullptr, "%s cmd failed: can't assign mesh, no texture loaded",
			LoadTextureComponent::loadMeshCmd))
			return false;

		// Serialize to JSON
		nap::rtti::JSONWriter writer;
		nap::rtti::ObjectList list = { path.getObject() };
		if (!serializeObjects(list, writer, error))
			return false;

		APIEventPtr load_mesh_event = std::make_unique<nap::APIEvent>(LoadTextureComponent::loadMeshCmd);
		load_mesh_event->addArgument<nap::APIString>(LoadTextureComponent::loadMeshArg1, writer.GetJSON());
		load_mesh_event->addArgument<nap::APIBool>(LoadTextureComponent::loadMeshArg2, frame);
		mRunner.sendEvent(std::move(load_mesh_event));
		mTrackedObject = path.getObject();
		return true;
	}


	void TexturePreviewPanel::clear()
	{
		// Send clear command to applet
		nap::APIEventPtr clear_tex_event = std::make_unique<nap::APIEvent>(LoadTextureComponent::clearCmd);
		mRunner.sendEvent(std::move(clear_tex_event));
		mLoadedTexture = nullptr;
	}


	void TexturePreviewPanel::propertyValueChanged(const PropertyPath& path)
	{
		// Bail if texture or mesh isn't loaded
		assert(path.isValid());
		if (path.getObject() != mTrackedObject)
			return;

		// Fetch loader and load
		auto loader = findLoader(path); assert(loader != nullptr);
		utility::ErrorState error;
		if (!loader(path, false, error))
			nap::Logger::error(error.toString());
	}


	void TexturePreviewPanel::objectRemoved(nap::rtti::Object* object)
	{
		if (object == mLoadedTexture)
			clear();
	}


	void TexturePreviewPanel::documentClosing(const QString& doc)
	{
		clear();
	}


	TexturePreviewPanel::Loader TexturePreviewPanel::findLoader(const PropertyPath& path)
	{
		using namespace std::placeholders;
		if(path.getObject()->get_type().is_derived_from(RTTI_OF(nap::Texture)))
			return std::bind(&TexturePreviewPanel::loadTexture, this, _1, _2, _3);

		if(path.getObject()->get_type().is_derived_from(RTTI_OF(nap::IMesh)))
			return std::bind(&TexturePreviewPanel::loadMesh, this, _1, _2, _3);

		return nullptr;
	}
}
