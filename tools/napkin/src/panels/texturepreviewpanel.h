/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "renderpanel.h"
#include "../applets/texturepreviewapicomponent.h"
#include "../applets/texturepreviewapplet.h"
#include "../appletrunner.h"
#include "../stagewidget.h"
#include "../thememanager.h"

namespace napkin
{
	// The applet this panel runs & embeds
	using TextureAppletRunner = TypedAppletRunner<TexturePreviewApplet>;

	/**
	 * Texture2D & Cubemap texture preview panel.
	 * 
	 * This panel hosts a NAP applet that utilizes the NAP render engine to preview textures.
	 * It supports both 2D textures and cube-maps, along with the option to assign custom meshes for preview purposes.
	 * The most recently loaded object is tracked for changes, and is automatically uploaded when a modification is detected.
	 * 
	 * Note that the applet runs on a separate thread, independent from Napkin.
	 * Communication is handled through API events, thread safe.
	 *
	 * The applet is started and added to the layout when a project is opened and initialization succeeds.
	 * If initialization fails the applet is not made visible -> see log for details.
	 */
	class TexturePreviewPanel : public StageWidget
	{
		Q_OBJECT
	public:
		// App to load (relative to napkin executable)
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
		nap::Texture*			mLoadedTexture = nullptr;	//< Current texture
		nap::rtti::Object*		mTrackedObject = nullptr;	//< Mesh or texture last loaded
		std::future<bool>		mFutureInit;

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

		// When theme changes
		void themeChanged(const Theme& theme);

		// Returns the load function for the given object
		using Loader = std::function<bool(const PropertyPath&, bool, nap::utility::ErrorState&)>;
		Loader findLoader(const PropertyPath& path);
	};
}
