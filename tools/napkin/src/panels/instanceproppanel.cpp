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
		// Get value as rtti variant
		auto override_variant = mAttrib.mValue->get_type().get_property_value(
			nap::rtti::instanceproperty::value, mAttrib.mValue);

		// Extract value using rtti
		QString value;
		if (override_variant.get_type().is_wrapper())
		{
			auto* obj = override_variant.extract_wrapped_value().get_value<nap::rtti::Object*>();
			assert(obj != nullptr);
			value = QString::fromStdString(obj->mID);
		}
		else
		{
			value = QString::fromStdString(override_variant.to_string());
		}

		assert(!value.isEmpty());
		return QString("%1 = %2").arg(mAttrib.mPath.c_str(), value);
	}
	case Qt::ForegroundRole:
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
	auto item = qitem_cast<InstancePropsItem*>(mTreeView.getSelectedItem());
	if (item != nullptr)
	{
		menu.addAction("Select Component Instance", this, [this, item]
			{
				auto root_entity_prop_item = qobject_cast<RootEntityPropItem*>(item->parentItem());
				assert(root_entity_prop_item);
				const nap::ComponentInstanceProperties& props = item->getProperties();

				auto rootEntity = &root_entity_prop_item->rootEntity();
				auto path = QString::fromStdString(props.mTargetComponent.toString());
				this->selectComponentRequested(rootEntity, path);
			});
	}
}


void InstancePropPanel::onModelChanged()
{
	mTreeView.getTreeView().expandAll();
}
