#pragma once

#include <QStandardItem>
#include <rtti/rtti.h>

/**
 * An item displaying an RTTI Type
 */
class RTTITypeItem : public QStandardItem {

public:
    RTTITypeItem(const nap::rtti::TypeInfo& type);


private:
    void refresh();

private:
    const nap::rtti::TypeInfo& type;
};
