/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "rttiitem.h"
#include "document.h"

// External Includes
#include <QWidget>
#include <QVBoxLayout>
#include <napqt/filtertreeview.h>
#include <instanceproperty.h>
#include <scene.h>

namespace napkin
{
	/**
	 * An item holding a nap::TargetAttribute
	 */
	class InstPropAttribItem : public RTTIItem
	{
		Q_OBJECT
	public:
		explicit InstPropAttribItem(nap::TargetAttribute& attrib);
		QVariant data(int role) const override;
	private:
		nap::RootEntity* rootEntity() const;

		nap::TargetAttribute& mAttrib;
	};


	/**
	 * An item holding nap::ComponentInstanceProperties
	 */
	class InstancePropsItem : public RTTIItem
	{
		Q_OBJECT
	public:
		explicit InstancePropsItem(nap::ComponentInstanceProperties& props);
		QVariant data(int role) const override;
		const nap::ComponentInstanceProperties& getProperties() const { return mProps; }

	private:
		nap::ComponentInstanceProperties& mProps;
	};


	/**
	 * An item holding a RootEntity
	 */
	class RootEntityPropItem : public RTTIItem
	{
		Q_OBJECT
	public :
		explicit RootEntityPropItem(nap::RootEntity& rootEntity);
		QVariant data(int role) const override;
		nap::RootEntity& rootEntity() const;
	private:
		nap::RootEntity& mRootEntity;
	};


	/**
	 * An item holding a Scene
	 */
	class InstPropSceneItem : public RTTIItem
	{
		Q_OBJECT
	public:
		explicit InstPropSceneItem(nap::Scene& scene);
		QVariant data(int role) const override;

	private:
		nap::Scene& mScene;
	};


	/**
	 * A model providing a tree of all instance properties in a scene
	 */
	class InstancePropModel : public QStandardItemModel
	{
	Q_OBJECT
	public:
		InstancePropModel();

	Q_SIGNALS:
		void sceneChanged();

	private:
		void populate();
		void onObjectRemoved(nap::rtti::Object* object);
		void onPropertyValueChanged(const PropertyPath& path);
		void onFileOpened(const QString& filename);
		void onFileClosing(const QString& filename);
	};


	/**
	 * This panel shows the currently set instance properties
	 */
	class InstancePropPanel : public QWidget
	{
		Q_OBJECT
	public:
		InstancePropPanel();

	Q_SIGNALS:
		void selectComponentRequested(nap::RootEntity* rootEntity, const QString& path);

	private:
		void menuHook(QMenu& menu);
		void onModelChanged();

		QVBoxLayout mLayout;
		nap::qt::FilterTreeView mTreeView;
		InstancePropModel mModel;
	};
}
