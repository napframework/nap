/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <QWidget>
#include <QWindow>
#include <QBoxLayout>

namespace napkin
{
	class RenderPanel : public QWidget
	{
		Q_OBJECT
	public:
		// Creates surface and adds it to this widget
		RenderPanel();

		// Deletes surface
		~RenderPanel();

	private:
		QWindow mNativeWindow;
		QWidget* mContainer = nullptr;
		QVBoxLayout mLayout;
	};
}
