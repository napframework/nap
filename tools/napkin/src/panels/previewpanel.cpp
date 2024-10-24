/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "previewpanel.h"
#include "../appcontext.h"
#include <apiservice.h>

namespace napkin
{
	PreviewPanel::PreviewPanel()
	{
		// Create render resources on project load
		connect(&AppContext::get(), &AppContext::projectLoaded, this, &PreviewPanel::init);
	}


	PreviewPanel::~PreviewPanel()
	{	
		mRunner.abort();
	}


	void PreviewPanel::closeEvent(QCloseEvent* event)
	{
		mRunner.abort();
		return QWidget::closeEvent(event);
	}


	void PreviewPanel::panelShown(napkin::RenderPanel& panel)
	{
		assert(mPanel != nullptr);
	}


	void PreviewPanel::init(const nap::ProjectInfo& info)
	{
		// Signals completion setup resources gui thread
		assert(mPanel == nullptr);

		// Create the window
		nap::utility::ErrorState error;
		mPanel = RenderPanel::create(mRunner, this, error);
		if (mPanel == nullptr)
		{
			nap::Logger::error(error.toString());
			return;
		}

		// Initialize and run the applet (core, services & application)
		auto preview_app = nap::utility::forceSeparator(nap::utility::getExecutableDir() + app);
		auto init_future = mRunner.start(preview_app, 60);

		// Don't install layout if initialization fails
		if (!init_future.get())
			return;

		// Hook up our widgets
		mAPIService = mRunner.getCore().getService<nap::APIService>();
		mLineEdit.connect(&mLineEdit, &QLineEdit::textChanged, this, &PreviewPanel::textChanged);

		// Install layout
		assert(layout() == nullptr);
		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(&mLineEdit);
		mLayout.addWidget(&mPanel->getWidget());
		setLayout(&mLayout);
	}


	void PreviewPanel::textChanged(const QString& text)
	{
		assert(mAPIService != nullptr);
		nap::APIEventPtr set_text_event = std::make_unique<nap::APIEvent>("PreviewSetText");
		set_text_event->addArgument<nap::APIString>("text", text.toStdString());

		nap::utility::ErrorState error;
		if (!mAPIService->sendEvent(std::move(set_text_event), &error))
			nap::Logger::error(error.toString());
	}
}
