#pragma once

#include <QItemDelegate>
#include <QComboBox>


#include <string>
#include <vector>

#include <QItemDelegate>

class QModelIndex;
class QWidget;
class QVariant;



class ComboBoxDelegate : public QItemDelegate
{
Q_OBJECT
public:
    ComboBoxDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    std::vector<std::string> Items;

};