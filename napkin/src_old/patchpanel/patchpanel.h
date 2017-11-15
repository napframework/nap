#include "patchscene.h"
#include "patchview.h"
#include <QDockWidget>

namespace Ui
{
	class PatchPanel;
}

// This is the main panel for flow graph programming
class PatchPanel : public QDockWidget
{
	Q_OBJECT

public:
	explicit PatchPanel(QWidget* parent = 0);
	~PatchPanel();

private slots:
	void onGlobalSelectionChanged(const QList<nap::Object*> objects);
	void onResetZoom();

private:
	Ui::PatchPanel* ui;
	PatchView mView;

};
