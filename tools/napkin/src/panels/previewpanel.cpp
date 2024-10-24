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

		// Setup control widgets
		mSpinbox.setRange(0, 120);
		mSpinbox.setValue(60);
		mSpinbox.setMinimumWidth(120);
		mSpinbox.setSuffix(" hz");
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
		auto init_future = mRunner.start(preview_app, mSpinbox.value());

		// Don't install layout if initialization fails
		if (!init_future.get())
			return;

		// Hook up our widgets
		mLineEdit.connect(&mLineEdit, &QLineEdit::textChanged, this, &PreviewPanel::textChanged);
		mSpinbox.connect(&mSpinbox, &QSpinBox::valueChanged, this, &PreviewPanel::freqChanged);

		// Create child widget layout
		assert(mMasterLayout.layout() == nullptr);
		mControlLayout.addWidget(&mLineEdit);
		mControlLayout.addWidget(&mSpinbox, 0, Qt::AlignRight);
		mControlLayout.setContentsMargins(0, 8, 0, 0);

		// Install layout
		assert(layout() == nullptr);
		mMasterLayout.setContentsMargins(0, 0, 0, 0);
		mMasterLayout.addLayout(&mControlLayout);
		mMasterLayout.addWidget(&mPanel->getWidget());
		setLayout(&mMasterLayout);
	}


	void PreviewPanel::textChanged(const QString& text)
	{
		nap::APIEventPtr set_text_event = std::make_unique<nap::APIEvent>("PreviewSetText");
		set_text_event->addArgument<nap::APIString>("text", text.toStdString());
		mRunner.sendEvent(std::move(set_text_event));
	}


	void PreviewPanel::freqChanged(int freq)
	{
		assert(freq > 0);
		mRunner.setFrequency(static_cast<nap::uint>(freq));
	}
}
