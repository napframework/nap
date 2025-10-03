/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <QEvent>
#include <inputevent.h>
#include <windowevent.h>
#include <QPoint>
#include <QWidget>
#include <QWindow>
#include <sdlhelpers.h>

namespace napkin
{
	/**
	 * Converts QT events into NAP Input Events
	 */
	class AppletEventConverter final
	{
	public:
		// Default constructor
		AppletEventConverter(SDL_Window* window, QWidget* container) :
			mWindow(window), mContainer(container) { }

		// Default destructor
		virtual ~AppletEventConverter() = default;

		/**
		 * Copy is not allowed
		 */
		AppletEventConverter(AppletEventConverter&) = delete;
		AppletEventConverter& operator=(const AppletEventConverter&) = delete;

		/**
		 * Move is not allowed
		 */
		AppletEventConverter(AppletEventConverter&&) = delete;
		AppletEventConverter& operator=(AppletEventConverter&&) = delete;

		/**
		* Utility function that checks if this is an input event (key, mouse, touch or controller)
		* @param qtEvent the Qt to verify
		* @return if this Qt event is an input event
		*/
		bool isInputEvent(const QEvent& qtEvent) const;

		/**
		* Utility function to translate a Qt event to a generic nap InputEvent
		* @param qtEvent The event to translate
		* @return Null if the Qt event is not an input event (or an unknown input event), the nap event otherwise
		*/
		nap::InputEventPtr translateInputEvent(const QEvent& qtEvent);

		/**
		* Utility function that checks if this is a key input event (key press down/up or text input)
		* @param qtEvent the qt event to check
		* @return if the Qt event is an input event
		*/
		bool isKeyEvent(const QEvent& qtEvent) const;

		/**
		* Utility function to translate an Qt event into a NAP key event
		* This call assumes that the given Qt event can be translated into a NAP key event!
		* Use isKeyEvent() to verify if the events are compatible
		* @param qtEvent the qt event to translate
		* @return a nap key event, nullptr if the event could not be translated
		*/
		nap::InputEventPtr translateKeyEvent(const QEvent& qtEvent);

		/**
		* Utility function that checks if the Qt event is a mouse input event
		* @param qtEvent the qt event to verify
		* @return if the event is a mouse input event
		*/
		bool isMouseEvent(const QEvent& qtEvent) const;

		/**
		* Utility function to translate an Qt event into a NAP mouse event.
		* This call assumes that the given Qt event can be translated into a NAP pointer (mouse) event!
		* Use isMouseEvent() to verify if the events are compatible.
		* @param qtEvent the qt mouse event to translate
		* @return a nap pointer event, nullptr if the event could not be translated
		*/
		nap::InputEventPtr translateMouseEvent(const QEvent& qtEvent);

		/**
		* Utility function that checks if the Qt event is a window event
		* @param qtEvent the qt event to verify
		* @return if the event is a window input event
		*/
		bool isWindowEvent(const QEvent& qtEvent) const;

		/**
		* Utility function to translate a Qt event into a NAP window event.
		* This call assumes that the given Qt event can be translated into a NAP window event!
		* Use isWindowEvent() to verify if the events are compatible.
		* @param qtEvent the qt window event to translate
		* @return a nap pointer event, nullptr if the event could not be translated
		*/
		nap::WindowEventPtr translateWindowEvent(const QEvent& qtEvent);

		/**
		 * Returns the platform specific pixel conversion ratio from QT to NAP.
		 * @return the platform specific pixel conversion ratio from QT to NAP.
		 */
		float getPixelRatio() const;

	private:
		SDL_Window* mWindow = nullptr;		///< SDL window handle
		QWidget* mContainer = nullptr;		///< QWidget handle
		QPoint mLocation = { -1, -1 };		///< Previous mouse location

		nap::InputEvent* translateQtKeyEvent(const QEvent& qtEvent, const nap::rtti::TypeInfo& eventType) const;
		nap::InputEvent* translateQtMouseEvent(const QEvent &qtEvent, QPoint &ioPrevious, const nap::rtti::TypeInfo &eventType) const;
		nap::WindowEvent* translateQtWindowEvent(const QEvent& qtEvent, const nap::rtti::TypeInfo& eventType);
	};
}

