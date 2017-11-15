#include "attributepanel.h"

#include <QSettings>

#include "../commands.h"


static const char* const HEADER_STATE = "AttributePanel_headerState";

AttributePanel::AttributePanel(QWidget* parent) : QDockWidget(parent)
{
	ui.setupUi(this);
	ui.treeView->setModel(&mModel);

	connect(&AppContext::get(), &AppContext::selectionChanged, this,
			&AttributePanel::onSelectionChanged);
	// Set initial empty selection
	onSelectionChanged(QList<nap::Object*>());
}

void AttributePanel::setObject(nap::AttributeObject* object)
{
	mModel.setObject(object);
	refresh();
}

void AttributePanel::onSelectionChanged(const QList<nap::Object*>& selection)
{
	if (selection.size() == 1 &&
		selection[0]->getTypeInfo().isKindOf<nap::AttributeObject>()) {
		nap::AttributeObject* object = (nap::AttributeObject*)selection[0];
		setObject(object);
	} else {
		setObject(nullptr);
	}
}


void AttributePanel::refresh()
{
	if (mModel.object()) {
		ui.dockWidgetContents->setEnabled(true);
	} else {
		ui.dockWidgetContents->setEnabled(false);
	}

	QSettings settings;
	ui.treeView->header()->restoreState(
		settings.value(HEADER_STATE).toByteArray());
}


void AttributePanel::hideEvent(QHideEvent* event)
{
	QSettings settings;
	settings.setValue(HEADER_STATE, ui.treeView->header()->saveState());
	QWidget::hideEvent(event);
}
