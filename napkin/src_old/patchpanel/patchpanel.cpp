#include "patchpanel.h"
#include "ui_patchpanel.h"
#include <nap/object.h>


PatchPanel::PatchPanel(QWidget* parent) : QDockWidget(parent), ui(new Ui::PatchPanel)
{
	ui->setupUi(this);

	ui->viewHolder->setLayout(new QVBoxLayout);
	ui->viewHolder->layout()->setMargin(0);
	ui->viewHolder->layout()->addWidget(&mView);

	connect(ui->btResetZoom, &QPushButton::clicked, this, &PatchPanel::onResetZoom);

//	mView.setScene(&mPatchScene);

	setEnabled(false);

	connect(&AppContext::get(), &AppContext::selectionChanged, this, &PatchPanel::onGlobalSelectionChanged);
}

PatchPanel::~PatchPanel() { delete ui; }


void PatchPanel::onGlobalSelectionChanged(const QList<nap::Object*> objects)
{
	nap::PatchComponent* patchComponent = AppContext::get().selected<nap::PatchComponent>();
	if (patchComponent) {
        setEnabled(true);
        patchComponent->getPatch().removed.connect([=](const nap::Object& ob) {
            setEnabled(false);
        });

        mView.setScene(new PatchScene(patchComponent->getPatch()));


	}

}

void PatchPanel::onResetZoom() {
	mView.resetZoom();
}
