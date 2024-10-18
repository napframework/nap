/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "../applet.h"

// External Includes
#include <QWidget>
#include <QWindow>
#include <QBoxLayout>
#include <renderwindow.h>

namespace napkin
{
	/**
	 * Creates and binds a QT widget container to a NAP render window.
	 */
	class RenderPanel : public QWidget
	{
		Q_OBJECT
	public:

		//////////////////////////////////////////////////////////////////////////

		/**
		 * Creates and binds a QT widget container to a NAP render window.
		 * @param applet the applet to bind the window to
		 * @param parent parent widget that owns the render panel
		 * @param the error if creation or binding fails
		 * @return the panel, nullptr if panel could not be created or bound
		 */
		static RenderPanel* create(napkin::Applet& applet, QWidget* parent, nap::utility::ErrorState& error);

		//////////////////////////////////////////////////////////////////////////

		/**
		 * @return render window
		 */
		nap::RenderWindow& getWindow()					{ assert(mRenderWindow != nullptr); return *mRenderWindow; }

		/**
		 * @return render window
		 */
		const nap::RenderWindow& getWindow() const		{ assert(mRenderWindow != nullptr); return *mRenderWindow; }

		/**
		 * @return QT window container
		 */
		QWidget& getContainer()							{ assert(mContainer != nullptr); return *mContainer; }

		/**
		 * @return QT window container
		 */
		const QWidget& getContainer() const				{ assert(mContainer != nullptr); return *mContainer; }

	Q_SIGNALS:
		// Signal called when the panel is shown
		void shown(RenderPanel& panel);

	protected:
		// Handle shown event
		virtual bool eventFilter(QObject* watched, QEvent* event) override;

	private:
		// Private constructor, call create instead
		RenderPanel(QWidget* container, nap::RenderWindow* window, QWidget* parent);

		QVBoxLayout		mLayout;
		QWidget*		mContainer = nullptr;
		nap::RenderWindow* mRenderWindow = nullptr;
	};
}

