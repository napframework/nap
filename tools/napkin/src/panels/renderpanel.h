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
#include <guiappeventhandler.h>

namespace napkin
{
	/**
	 * Runs a nap application inside a widget
	 */
	class RenderPanel : public QWidget
	{
		Q_OBJECT
	public:

		static RenderPanel* create(napkin::Applet& applet, QWidget* parent, nap::utility::ErrorState& error);

	protected:
		virtual bool eventFilter(QObject* watched, QEvent* event) override;

	private:
		// Render panel embeds the container
		RenderPanel(QWidget* container, nap::rtti::ObjectPtr<nap::RenderWindow> window, QWidget* parent);

		QVBoxLayout mLayout;
		QWidget* mContainer = nullptr;
		nap::rtti::ObjectPtr<nap::RenderWindow> mRenderWindow = nullptr;
	};
}

