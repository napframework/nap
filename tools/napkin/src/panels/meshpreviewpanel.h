/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "renderpanel.h"
#include "../applets/meshpreviewapplet.h"
#include "../appletrunner.h"
#include "../stagewidget.h"
#include "../thememanager.h"

// External includes
#include <QLineEdit>
#include <QSpinBox>
#include <apiservice.h>
#include <mesh.h>
#include <material.h>

namespace napkin
{
	// The applet this panel runs & embeds
	using MeshPreviewAppletRunner = TypedAppletRunner<MeshPreviewApplet> ;

	/**
	 * Allows for previewing material and meshes
	 */
	class MeshPreviewPanel : public StageWidget
	{
		Q_OBJECT
	public:
		// App to load (relative to executable)
		static constexpr const char* app = "/resources/applets/meshpreview/app.json";

		// Creates the surface and adds it to this widget
		MeshPreviewPanel(QWidget* parent = nullptr);

		// Ensures applet stops running
		~MeshPreviewPanel();

		/**
		 * @return if the preview applet is initialized
		 */
		bool initialized() const									{ return mPanel != nullptr; }

	protected:
		// Called by the main window when the widget is closed
		virtual void closeEvent(QCloseEvent* event) override;

		// Loads the resource
		virtual bool onLoadPath(const PropertyPath& path, nap::utility::ErrorState& error) override;

	private:
		RenderPanel*			mPanel = nullptr;		//< NAP compatible Qt render window
		MeshPreviewAppletRunner	mRunner;				//< Application that is run
		QVBoxLayout				mMasterLayout;			//< Master widget layout
		nap::rtti::Object*		mMesh = nullptr;	//< Currently loaded mesh
		std::future<bool>		mInitFuture;			

		// Creates the app and links the window
		void init(const nap::ProjectInfo& info);

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
	};
}
