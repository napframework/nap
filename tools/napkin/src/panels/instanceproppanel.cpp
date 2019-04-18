#include "instanceproppanel.h"

#include <appcontext.h>
#include <rtti/object.h>
#include "naputils.h"

using namespace napkin;

InstPropAttribItem::InstPropAttribItem(nap::TargetAttribute& attrib) : QStandardItem(), mAttrib(attrib)
{
	setEditable(false);
	setText(QString::fromStdString(attrib.mPath));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InstancePropsItem::InstancePropsItem(nap::ComponentInstanceProperties& props) : QStandardItem(), mProps(props)
{
	setEditable(false);
	setText(QString::fromStdString(props.mTargetComponent.getInstancePath()));
	for (auto& a : props.mTargetAttributes)
		appendRow(new InstPropAttribItem(a));
}

QVariant InstancePropsItem::data(int role) const
{
	if (role == Qt::ForegroundRole)
	{
		auto bgcol = Qt::white;
		auto fgcol = QStandardItem::data(role).value<QColor>();
		return QVariant::fromValue<QColor>(nap::qt::lerpCol(bgcol, fgcol, 0.5));
	}
	return QStandardItem::data(role);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RootEntityPropItem::RootEntityPropItem(nap::RootEntity& rootEntity) : QStandardItem(), mRootEntity(rootEntity)
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
		auto bgcol = Qt::white;
		auto fgcol = QStandardItem::data(role).value<QColor>();
		return QVariant::fromValue<QColor>(nap::qt::lerpCol(bgcol, fgcol, 0.5));
	}
	return QStandardItem::data(role);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InstPropSceneItem::InstPropSceneItem(nap::Scene& scene) : QStandardItem(), mScene(scene)
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
		auto bgcol = Qt::white;
		auto fgcol = QStandardItem::data(role).value<QColor>();
		return QVariant::fromValue<QColor>(nap::qt::lerpCol(bgcol, fgcol, 0.5));
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
	connect(doc, &Document::objectChanged, [this](nap::rtti::Object* object) {
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
}

void InstancePropPanel::onModelChanged()
{
	mTreeView.getTreeView().expandAll();
}


