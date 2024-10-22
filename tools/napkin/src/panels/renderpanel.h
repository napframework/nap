/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "appleteventconverter.h"

// External Includes
#include <QWidget>
#include <QWindow>
#include <QBoxLayout>
#include <renderwindow.h>

namespace napkin
{
	// Forward declares
	class AppletRunner;

	/**
	 * Creates and binds a QT widget container to a NAP render window.
	 */
	class RenderPanel : public QWidget
	{
		Q_OBJECT
	public:
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Creates QT widget container for a NAP render window.
		 * @param applet the applet to bind the window to
		 * @param parent parent widget that owns the render panel
		 * @param the error if creation or binding fails
		 * @return the panel, nullptr if panel could not be created
		 */
		static RenderPanel* create(napkin::AppletRunner& applet, QWidget* parent, nap::utility::ErrorState& error);

		//////////////////////////////////////////////////////////////////////////

		/**
		 * @return QT window container
		 */
		QWidget& getContainer()							{ assert(mContainer != nullptr); return *mContainer; }

		/**
		 * @return QT window container
		 */
		const QWidget& getContainer() const				{ assert(mContainer != nullptr); return *mContainer; }

		/**
		 * Called when the panel is shown
		 */
		void showEvent(QShowEvent* event) override;

	protected:
		// Handle shown event
		virtual bool eventFilter(QObject* watched, QEvent* event) override;

	private:
		// Private constructor, call create instead
		RenderPanel(QWidget* container, SDL_Window* window, QWidget* parent, AppletRunner& applet);

		QVBoxLayout				mLayout;
		QWidget*				mContainer = nullptr;
		SDL_Window*				mWindow = nullptr;
		AppletRunner&			mApplet;
		QString					mName;
		AppletEventConverter	mConverter;
	};
}

