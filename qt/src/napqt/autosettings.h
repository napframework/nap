/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <cassert>
#include <memory>

#include <QWidget>
#include <QSettings>
#include <QMainWindow>
#include <QHeaderView>
#include <QSplitter>

namespace nap
{
	namespace qt
	{
		class WidgetStorerBase;

		/**
		 * An instance of this class allows one to persist an entire tree of QWidget instances at once.
		 * Examples are: QDockWidget layout, QHeaderView section sizes, QTreeView expansion states, etc.
		 * In order to add custom behaviour, subclass WidgetStorer.
		 */
		class AutoSettings
		{
		public:
			/**
			 * Autosettings is a Singleton, get it here.
			 */
			static AutoSettings& get();

			/**
			 * Store the given widget and its children recursively
			 *
			 * @param w The widget and its subtree to store
			 */
			void store(QWidget& w) const;

			/**
			 * Restore the given widget and its children recursively
			 *
			 * @param w The widget and its subtree to restore.
			 */
			void restore(QWidget& w) const;

			/**
			 * Register a custom storer, note that the order of registration matters:
			 * When finding a storer for a particular widget,
			 * it will do so by iterating through the registered list
			 * and picking the first compatible type T
			 */
			template<typename T>
			void registerStorer()
			{
				registerStorer(std::make_unique<T>());
			}

			/**
			 * Exclude the specified widget instance from storing or restoring its settings
			 * @param widget
			 */
			void exclude(QWidget* widget)
			{
				mExclusions << widget;
			}

			/**
			 * Register a WidgetStorer. Autosettings will automatically recognize and store/restore
			 * the settings using the provided instance.
			 */
			void registerStorer(std::unique_ptr<WidgetStorerBase> s);
		private:
			AutoSettings();
			AutoSettings(AutoSettings const&);
			void operator=(AutoSettings const&);

			void registerDefaults();

			void storeRecursive(QWidget& w, QSettings& s) const;
			void restoreRecursive(QWidget& w, const QSettings& s) const;
			const QString ensureHasName(QObject& w) const;
			QString uniqeObjectName(QWidget& w) const;

			WidgetStorerBase* findStorer(const QWidget& w) const;

			std::vector<std::unique_ptr<WidgetStorerBase>> mStorers;
			QList<QWidget*> mExclusions;
		};

		/**
		 * Base class for WidgetStorer, used for polymorphism.
		 * Use the templated WidgetStorer to implement custom storers.
		 */
		class WidgetStorerBase
		{
		public:
			virtual ~WidgetStorerBase() = default;

			virtual void storeWidget(const QWidget& widget, const QString& key, QSettings& s) const = 0;
			virtual void restoreWidget(QWidget& widget, const QString& key, const QSettings& s) const = 0;
			virtual bool canStore(const QWidget& widget) const = 0;
		};

		/**
		 * Derive from this class to implement a custom persistence Widget storer,
		 * then register an instance with AutoSettings to have instances of T stored/restored.
		 *
		 * @tparam T The type of widget we want to store.
		 */
		template<typename T>
		class WidgetStorer : public WidgetStorerBase
		{
		public:
			WidgetStorer() = default;
			~WidgetStorer() override = default;

			/**
			 * Store a widget.
			 * @param widget The QWidget to get the settings from
			 * @param key A unique string identifying the provided QWidget
			 * @param s The QSettings instance on which to store the settings
			 */
			virtual void store(const T& widget, const QString& key, QSettings& s) const = 0;

			/**
			 * Restore a widget
			 * @param widget The QWidget to apply the settings to
			 * @param key A unique string identifying the provided QWidget
			 * @param s The QSetttings instance to get the settings from
			 */
			virtual void restore(T& widget, const QString& key, const QSettings& s) const = 0;

			void storeWidget(const QWidget& widget, const QString& key, QSettings& s) const override
			{
				auto w = qobject_cast<const T*>(&widget);
				assert(w);
				store(*w, key, s);
			}

			void restoreWidget(QWidget& widget, const QString& key, const QSettings& s) const override
			{
				auto w = qobject_cast<T*>(&widget);
				assert(w);
				restore(*w, key, s);
			}

			bool canStore(const QWidget& widget) const override
			{
				return qobject_cast<const T*>(&widget) != nullptr;
			}
		};

		class MainWindowWidgetStorer : public WidgetStorer<QMainWindow>
		{
		public:
			void store(const QMainWindow& widget, const QString& key, QSettings& s) const override
			{
				s.setValue(key + "_GEO", widget.saveGeometry());
				s.setValue(key + "_STATE", widget.saveState());
			}

			void restore(QMainWindow& widget, const QString& key, const QSettings& s) const override
			{
				widget.restoreGeometry(s.value(key + "_GEO").toByteArray());
				widget.restoreState(s.value(key + "_STATE").toByteArray());

			}
		};


		class HeaderViewWidgetStorer : public WidgetStorer<QHeaderView>
		{
		public:
			void store(const QHeaderView& widget, const QString& key, QSettings& s) const override
			{
				s.setValue(key + "_GEO", widget.saveGeometry());
				s.setValue(key + "_STATE", widget.saveState());
			}

			void restore(QHeaderView& widget, const QString& key, const QSettings& s) const override
			{
				widget.restoreGeometry(s.value(key + "_GEO").toByteArray());
				widget.restoreState(s.value(key + "_STATE").toByteArray());
			}
		};

		class SplitterStorer : public WidgetStorer<QSplitter>
		{
		public:
			void store(const QSplitter& widget, const QString& key, QSettings& s) const override
			{
				s.setValue(key + "_GEO", widget.saveGeometry());
				s.setValue(key + "_STATE", widget.saveState());
			}

			void restore(QSplitter& widget, const QString& key, const QSettings& s) const override
			{
				widget.restoreGeometry(s.value(key + "_GEO").toByteArray());
				widget.restoreState(s.value(key + "_STATE").toByteArray());
			}
		};

	} // namespace qt

} // namespace nap
