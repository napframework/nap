/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "texturepreviewpanel.h"
#include "../appcontext.h"

// External includes
#include <rtti/jsonwriter.h>
#include <trianglemesh.h>
#include <image.h>
#include <rendertexturecube.h>

namespace napkin
{
	// Widget name
	static constexpr const char* sPanelName = "Texture Preview";

	// Included types
	static StageOption::Types getTypes()
	{
		return
		{
			RTTI_OF(nap::Texture2D),
			RTTI_OF(nap::TextureCube),
			RTTI_OF(nap::IMesh)
		};
	}

	// Explicit excluded types
	static StageOption::Types getExcludedTypes()
	{
		return
		{
			 RTTI_OF(nap::TriangleMesh),
			 RTTI_OF(nap::Image),
			 RTTI_OF(nap::RenderTexture2D),
			 RTTI_OF(nap::DepthRenderTexture2D),
			 RTTI_OF(nap::RenderTextureCube),
			 RTTI_OF(nap::DepthRenderTextureCube)
		};
	}


	TexturePreviewPanel::TexturePreviewPanel(QWidget* parent) : StageWidget(sPanelName,
		getTypes(), getExcludedTypes(), RTTI_OF(nap::Texture), parent)
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
		// Don't do anything if textures aren't supported
		if (!isSupported(info))
			return;

		// Should only be initialized once, after the project is loaded
		assert(mPanel == nullptr);

		// Create the nap compatible render window
		nap::utility::ErrorState error;
		mPanel = RenderPanel::create(mRunner, *this, error);
		if (!error.check(mPanel != nullptr, "Unable to create '%s' render panel", sPanelName))
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Initializing the applet (core, services & application)
		auto preview_app = nap::utility::forceSeparator(nap::utility::getExecutableDir() + app);
		assert(!mFutureInit.valid());
		mFutureInit = mRunner.start(preview_app, info, true);

		// Let the applet initialize on it's own thread -> install next frame
		QTimer::singleShot(0, [this]()
			{
				// Wait until initialized and bail on failure
				assert(mFutureInit.valid());
				if (!mFutureInit.get())
				{
					nap::Logger::error("'%s' initialization failed, check the log for more details", sPanelName);
					return;
				}

				// Sync theme
				const auto* theme = AppContext::get().getThemeManager().getCurrentTheme();
				assert(theme != nullptr);
				themeChanged(*theme);

				// Install window into this widget
				assert(layout() == nullptr);
				mLayout.setContentsMargins(0, 0, 0, 0);
				mLayout.addWidget(&mPanel->getWidget());
				setLayout(&mLayout);

				// Listen to property changes
				connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &TexturePreviewPanel::propertyValueChanged);
				connect(&AppContext::get(), &AppContext::objectRemoved, this, &TexturePreviewPanel::objectRemoved);
				connect(&AppContext::get(), &AppContext::documentClosing, this, &TexturePreviewPanel::documentClosing);
				connect(&AppContext::get().getThemeManager(), &ThemeManager::themeChanged, this, &TexturePreviewPanel::themeChanged);
			});
	}


	void TexturePreviewPanel::closeEvent(QCloseEvent* event)
	{
		mRunner.abort();
		QWidget::closeEvent(event);
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
		APIEventPtr load_tex_event = std::make_unique<nap::APIEvent>(TexturePreviewAPIComponent::loadTextureCmd);
		load_tex_event->addArgument<nap::APIString>(TexturePreviewAPIComponent::loadTextureArg1, writer.GetJSON());
		load_tex_event->addArgument<nap::APIBool>(TexturePreviewAPIComponent::loadTextureArg2, frame);
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
		if (mLoadedTexture == nullptr)
		{
			nap::Logger::warn("Unable to assign mesh: a texture must be loaded first");
			return true;
		}

		// Serialize to JSON
		nap::rtti::JSONWriter writer;
		nap::rtti::ObjectList list = { path.getObject() };
		if (!serializeObjects(list, writer, error))
			return false;

		APIEventPtr load_mesh_event = std::make_unique<nap::APIEvent>(TexturePreviewAPIComponent::loadMeshCmd);
		load_mesh_event->addArgument<nap::APIString>(TexturePreviewAPIComponent::loadMeshArg1, writer.GetJSON());
		load_mesh_event->addArgument<nap::APIBool>(TexturePreviewAPIComponent::loadMeshArg2, frame);
		mRunner.sendEvent(std::move(load_mesh_event));
		mTrackedObject = path.getObject();
		return true;
	}


	void TexturePreviewPanel::clear()
	{
		// Send clear command to applet
		nap::APIEventPtr clear_tex_event = std::make_unique<nap::APIEvent>(TexturePreviewAPIComponent::clearCmd);
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


	void TexturePreviewPanel::themeChanged(const Theme& theme)
	{
		APIEventPtr change_theme_cmd = std::make_unique<nap::APIEvent>(TexturePreviewAPIComponent::changeThemeCmd);
		change_theme_cmd->addArgument<nap::APIString>(TexturePreviewAPIComponent::changeThemeArg1, theme.getName().toStdString());
		mRunner.sendEvent(std::move(change_theme_cmd));
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

