/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "appcontext.h"

// External Includes
#include <QWidget>
#include <QWindow>
#include <QBoxLayout>
#include <renderwindow.h>

namespace napkin
{
	class RenderPanel : public QWidget
	{
		Q_OBJECT
	public:
		// Creates surface and adds it to this widget
		RenderPanel();

	protected:
		virtual bool eventFilter(QObject* watched, QEvent* event) override;

	private:
		QWindow mNativeWindow;
		QVBoxLayout mLayout;
		QWidget* mContainer = nullptr;
		std::unique_ptr<nap::RenderWindow> mRenderWindow = nullptr;
		void projectLoaded(const nap::ProjectInfo& info);
		void createResources();

		void draw();
	};
}
