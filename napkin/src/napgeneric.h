#pragma once

#include <QStandardItem>
#include <rtti/rtti.h>


class TypeItem : public QStandardItem {

public:
    TypeItem(const nap::rtti::TypeInfo& type);


private:
    void refresh();

private:
    const nap::rtti::TypeInfo& type;
};
