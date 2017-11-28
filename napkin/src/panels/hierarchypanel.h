#pragma once

#include <rtti/rtti.h>
#include <rtti/rttiobject.h>

#include "generic/filtertreeview.h"
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWidget>

class TypeModel : public QStandardItemModel
{
public:
	TypeModel();

private:
	void refresh();
};


class HierarchyPanel : public QWidget
{
public:
	HierarchyPanel();

private:
	QVBoxLayout mLayout;
	FilterTreeView mTreeView;
	TypeModel mModel;
};