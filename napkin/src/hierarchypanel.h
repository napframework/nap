#pragma once


// Qt's slots keyword overlaps with Python's

#include <rtti/rtti.h>
#include <rtti/rttiobject.h>

#include <QWidget>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include "generic/filtertreeview.h"



class TypeModel : public QStandardItemModel {
public:
    TypeModel();

private:
    void refresh();

};


class HierarchyPanel : public QWidget {
public:
    HierarchyPanel();

private:
    QVBoxLayout mLayout;
    FilterTreeView mTreeView;
    TypeModel mModel;
};