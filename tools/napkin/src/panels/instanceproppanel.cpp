/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "instanceproppanel.h"
#include "naputils.h"

#include <appcontext.h>
#include <rtti/object.h>
#include <typeconversion.h>

using namespace napkin;

InstPropAttribItem::InstPropAttribItem(nap::TargetAttribute& attrib) : mAttrib(attrib)
{
	setEditable(false);

}

QVariant InstPropAttribItem::data(int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
	{
		QString path = QString::fromStdString(mAttrib.mPath);
		auto instPropsItem = qobject_cast<InstancePropsItem*>(parentItem());
		assert(instPropsItem);
		auto compPath = instPropsItem->props().mTargetComponent.getInstancePath();

		PropertyPath propPath(compPath, mAttrib.mPath, *AppContext::get().getDocument());
		QString val;
		if (propPath.isPointer())
		{
			auto pointee = propPath.getPointee();
			val = pointee ? QString::fromStdString(pointee->mID) : "NULL";
		}
		else
		{
			bool ok;
			val = QString::fromStdString(propPath.getValue().to_string(&ok));
		}
		return QString("%1 = %2").arg(path, val);
	}
	case Qt::TextColorRole:
	{
		return AppContext::get().getThemeManager().getColor(theme::color::instancePropertyOverride);
	}
	default:
		return QStandardItem::data(role);
	}
}


nap::RootEntity* InstPropAttribItem::rootEntity() const
{
	auto parentItem = this->parentItem();
	while (parentItem)
	{
		if (auto rootEntItem = qobject_cast<RootEntityPropItem*>(this->parentItem()))
		{
			return &rootEntItem->rootEntity();
		}
		parentItem = this->parentItem();
	}
	return nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InstancePropsItem::InstancePropsItem(nap::ComponentInstanceProperties& props) : mProps(props)
{
	setEditable(false);
	setText(QString::fromStdString(props.mTargetComponent.getInstancePath()));
	for (auto& a : props.mTargetAttributes)
	{
		appendRow(new InstPropAttribItem(a));
	}
}


QVariant InstancePropsItem::data(int role) const
{
	if (role == Qt::ForegroundRole)
	{
		auto& themeMgr = AppContext::get().getThemeManager();
		return QVariant::fromValue<QColor>(themeMgr.getColor(theme::color::dimmedItem));
	}
	return QStandardItem::data(role);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RootEntityPropItem::RootEntityPropItem(nap::RootEntity& rootEntity) : mRootEntity(rootEntity)
{
	setEditable(false);
	setText(QString::fromStdString(rootEntity.mEntity->mID));
	for (auto& p : rootEntity.mInstanceProperties)
		appendRow(new InstancePropsItem(p));
}

QVariant RootEntityPropItem::data(int role) const
{
	if (role == Qt::ForegroundRole)
	{
		auto& themeMgr = AppContext::get().getThemeManager();
		return QVariant::fromValue<QColor>(themeMgr.getColor(theme::color::dimmedItem));
	}
	return QStandardItem::data(role);
}

nap::RootEntity& RootEntityPropItem::rootEntity() const
{
	return mRootEntity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InstPropSceneItem::InstPropSceneItem(nap::Scene& scene) : mScene(scene)
{
	setEditable(false);
	setText(QString::fromStdString(scene.mID));
	for (auto& e : scene.mEntities)
		appendRow(new RootEntityPropItem(e));
}

QVariant InstPropSceneItem::data(int role) const
{
	if (role == Qt::ForegroundRole)
	{
		auto& themeMgr = AppContext::get().getThemeManager();
		return QVariant::fromValue<QColor>(themeMgr.getColor(theme::color::dimmedItem));
	}
	return QStandardItem::data(role);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
InstancePropModel::InstancePropModel()
{
	auto ctx = &AppContext::get();
	connect(ctx, &AppContext::documentChanged, this, &InstancePropModel::onDocumentChanged);
}

void InstancePropModel::onDocumentChanged()
{
	auto doc = AppContext::get().getDocument();
	connect(doc, &Document::objectChanged, [this](nap::rtti::Object* object)
	{
		if (object->get_type().is_derived_from<nap::Scene>())
			onSceneChanged();
	});
	onSceneChanged();
}

void InstancePropModel::onSceneChanged()
{
	removeRows(0, rowCount());
	for (auto scene : AppContext::get().getDocument()->getObjects<nap::Scene>())
	{
		appendRow(new InstPropSceneItem(*scene));
	}
	sceneChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InstancePropPanel::InstancePropPanel()
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
	mTreeView.getTreeView().setHeaderHidden(true);

	connect(&mModel, &InstancePropModel::sceneChanged, this, &InstancePropPanel::onModelChanged);
	mTreeView.setMenuHook(std::bind(&InstancePropPanel::menuHook, this, std::placeholders::_1));
}

void InstancePropPanel::menuHook(QMenu& menu)
{
	auto instPropsItem = mTreeView.getSelectedItem<InstancePropsItem>();
	if (instPropsItem)
	{
		menu.addAction("Select Component Instance", this, &InstancePropPanel::onSelectComponentInstance);
	}
}

void InstancePropPanel::onModelChanged()
{
	mTreeView.getTreeView().expandAll();
}

void InstancePropPanel::onSelectComponentInstance()
{
	auto instPropsItem = mTreeView.getSelectedItem<InstancePropsItem>();
	auto rootEntityPropItem = qobject_cast<RootEntityPropItem*>(instPropsItem->parentItem());
	assert(rootEntityPropItem);
	const nap::ComponentInstanceProperties& props = instPropsItem->props();

	auto rootEntity = &rootEntityPropItem->rootEntity();
	auto path = QString::fromStdString(props.mTargetComponent.toString());
	selectComponentRequested(rootEntity, path);
}
