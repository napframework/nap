#pragma once

#include <cassert>
#include <memory>

#include <QWidget>
#include <QSettings>

namespace napkin
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
		 *
		 * @param s The storer to register
		 */
		void registerStorer(WidgetStorerBase* s);

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
	};

	/**
	 * Base class for WidgetStorer, used for polymorphism.
	 * Use the templated WidgetStorer to implement custom storers.
	 */
	class WidgetStorerBase
	{
	public:
		~WidgetStorerBase() = default;

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

		/**
		 * Store a widget.
		 * @param widget The QWidget to get the settings from
		 * @param key A unique string identifying the provided QWidget
		 * @param s The QSettings instance on which to store the settings
		 */
		void store(const T& widget, const QString& key, QSettings& s) const;

		/**
		 * Restore a widget
		 * @param widget The QWidget to apply the settings to
		 * @param key A unique string identifying the provided QWidget
		 * @param s The QSetttings instance to get the settings from
		 */
		void restore(T& widget, const QString& key, const QSettings& s) const;

		void storeWidget(const QWidget& widget, const QString& key, QSettings& s) const override
		{
			const T* w = dynamic_cast<const T*>(&widget);
			assert(w);
			store(*w, key, s);
		}

		void restoreWidget(QWidget& widget, const QString& key, const QSettings& s) const override
		{
			T* w = dynamic_cast<T*>(&widget);
			assert(w);
			restore(*w, key, s);
		}

		bool canStore(const QWidget& widget) const override
		{
			return dynamic_cast<const T*>(&widget) != nullptr;
		}
	};


}