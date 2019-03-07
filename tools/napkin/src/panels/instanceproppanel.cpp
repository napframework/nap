#include <appcontext.h>
#include <rtti/object.h>
#include "instanceproppanel.h"


using namespace napkin;

InstPropAttribItem::InstPropAttribItem(nap::TargetAttribute& attrib) : QStandardItem(), mAttrib(attrib)
{
	setText(QString::fromStdString(attrib.mPath));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InstancePropsItem::InstancePropsItem(nap::ComponentInstanceProperties& props) : QStandardItem(), mProps(props)
{
	setText(QString::fromStdString(props.mTargetComponent->mID));
	for (auto& a : props.mTargetAttributes)
		appendRow(new InstPropAttribItem(a));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RootEntityPropItem::RootEntityPropItem(nap::RootEntity& rootEntity) : QStandardItem(), mRootEntity(rootEntity)
{
	setText(QString::fromStdString(rootEntity.mEntity->mID));
	for (auto& p : rootEntity.mInstanceProperties)
		appendRow(new InstancePropsItem(p));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

InstPropSceneItem::InstPropSceneItem(nap::Scene& scene) : QStandardItem(), mScene(scene)
{
	setText(QString::fromStdString(scene.mID));
	for (auto& e : scene.mEntities)
		appendRow(new RootEntityPropItem(e));
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

	connect(&mModel, &InstancePropModel::sceneChanged, this, &InstancePropPanel::onModelChanged);
}

void InstancePropPanel::onModelChanged()
{
	mTreeView.getTreeView().expandAll();
}


