#pragma once

#include "attributemodel.h"
#include "ui_attributepanel.h"
#include <QDockWidget>
#include <nap/logger.h>


namespace Ui
{
	class AttributePanel;
}

class AttributePanel : public QDockWidget
{
	Q_OBJECT
public:
	explicit AttributePanel(QWidget* parent = 0);

	~AttributePanel() {}

	void setObject(nap::AttributeObject* object);

protected:
    void hideEvent(QHideEvent *event);


private slots:
	void onSelectionChanged(const QList<nap::Object*>& selection);
    void refresh();

private:
	Ui::AttributePanel ui;
	AttributeModel mModel;

};