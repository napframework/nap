/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "appcontext.h"
#include "appletlauncher.h"
#include "apps/renderpreviewapp.h"

// External Includes
#include <QWidget>
#include <QWindow>
#include <QTImer>
#include <QBoxLayout>
#include <renderwindow.h>
#include <guiappeventhandler.h>

namespace napkin
{
	/**
	 * Window created using an external SDL window handle
	 */
	class RenderWindowObjectCreator : public nap::rtti::IObjectCreator
	{
	public:
		/**
		*/
		RenderWindowObjectCreator(nap::Core& core, void* windowHandle) :
			mCore(core), mWindowHandle(windowHandle) { }

		/**
		* @return type to create
		*/
		nap::rtti::TypeInfo getTypeToCreate() const override { return RTTI_OF(nap::RenderWindow); }

		/**
		* @return Constructs the resource using as a first argument the object stored in this class
		*/
		virtual nap::rtti::Object* create() override { return new nap::RenderWindow(mCore, mWindowHandle); }

	private:
		void* mWindowHandle = nullptr;
		nap::Core& mCore;
	};


	/**
	 * Runs a nap application inside a widget
	 */
	class RenderPanel : public QWidget
	{
		Q_OBJECT
	public:
		// Creates surface and adds it to this widget
		RenderPanel();

		// Free resources
		virtual ~RenderPanel() override;

	protected:
		virtual bool eventFilter(QObject* watched, QEvent* event) override;

	private:
		QWindow mNativeWindow;
		QVBoxLayout mLayout;
		QWidget* mContainer = nullptr;
		nap::ObjectPtr<nap::RenderWindow> mRenderWindow = nullptr;
		void projectLoaded(const nap::ProjectInfo& info);
		void createResources();

		using PreviewApplet = napkin::AppletLauncher<nap::RenderPreviewApp, nap::GUIAppEventHandler>;
		PreviewApplet mApplet;

		QTimer mTimer;
		void timerEvent();
	};
}


