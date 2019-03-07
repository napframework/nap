#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <napqt/filtertreeview.h>

namespace napkin
{

	class InstPropAttribItem : public QStandardItem
	{
	public:
		explicit InstPropAttribItem(nap::TargetAttribute& attrib);

	private:
		nap::TargetAttribute& mAttrib;
	};

	class InstancePropsItem : public QStandardItem
	{
	public:
		explicit InstancePropsItem(nap::ComponentInstanceProperties& props);
	private:
		nap::ComponentInstanceProperties& mProps;
	};

	class RootEntityPropItem : public QStandardItem
	{
	public :
		explicit RootEntityPropItem(nap::RootEntity& rootEntity);
	private:
		nap::RootEntity& mRootEntity;
	};

	class InstPropSceneItem : public QStandardItem
	{
	public:
		explicit InstPropSceneItem(nap::Scene& scene);
	private:
		nap::Scene& mScene;
	};

	class InstancePropModel : public QStandardItemModel
	{
	Q_OBJECT
	public:
		InstancePropModel();
	Q_SIGNALS:
		void sceneChanged();
	private:
		void onDocumentChanged();
		void onSceneChanged();
	};

	class InstancePropPanel : public QWidget
	{
	public:
		InstancePropPanel();

	private:
		void onModelChanged();

		QVBoxLayout mLayout;
		nap::qt::FilterTreeView mTreeView;
		InstancePropModel mModel;
	};
}
