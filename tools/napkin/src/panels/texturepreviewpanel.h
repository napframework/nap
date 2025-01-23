/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "renderpanel.h"
#include "../applets/loadtexturecomponent.h"
#include "../applets/texturepreviewapplet.h"
#include "../appletrunner.h"
#include "../stagewidget.h"

// External includes
#include <rendertexture2d.h>

namespace napkin
{
	// The applet this panel runs & embeds
	using TextureAppletRunner = TypedAppletRunner<TexturePreviewApplet>;

	/**
	 * Allows for previewing material and meshes
	 */
	class TexturePreviewPanel : public StageWidget
	{
		Q_OBJECT
	public:
		// App to load (relative to executable)
		static constexpr const char* app = "/resources/applets/texturepreview/app.json";

		// Creates the surface and adds it to this widget
		TexturePreviewPanel(QWidget* parent = nullptr);

		// Ensures applet stops running
		~TexturePreviewPanel();

		/**
		 * @return if the preview applet is initialized
		 */
		bool initialized() const						{ return mPanel != nullptr; }

	protected:

		// Called by the main window when the widget is closed
		virtual void closeEvent(QCloseEvent* event) override;

		// Loads the texture -> path must point to type Texture2D
		virtual bool onLoadPath(const PropertyPath& path, utility::ErrorState& error) override;

	private:
		RenderPanel*			mPanel = nullptr;			//< NAP compatible Qt render window
		TextureAppletRunner		mRunner;					//< Application that is run
		QVBoxLayout				mLayout;					//< Widget layout
		bool					mInitialized;				//< If the panel is initialized
		nap::Texture*			mLoadedTexture = nullptr;	//< Current texture
		nap::rtti::Object*		mTrackedObject = nullptr;	//< Mesh or texture last loaded

		// Creates the app and links the window
		void init(const nap::ProjectInfo& info);

		// Load a texture
		bool loadTexture(const PropertyPath& path, bool frame, nap::utility::ErrorState& error);

		// Load a mesh
		bool loadMesh(const PropertyPath& path, bool frame, nap::utility::ErrorState& error);

		// Clear selection
		void clear();

		// When a property value changes
		void propertyValueChanged(const PropertyPath& path);

		// When an object is removed
		void objectRemoved(nap::rtti::Object* object);

		// When document is closing
		void documentClosing(const QString& doc);

		// Returns the load function for the given object
		using Loader = std::function<bool(const PropertyPath&, bool, nap::utility::ErrorState&)>;
		Loader findLoader(const PropertyPath& path);
	};
}


