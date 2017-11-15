#pragma once

#include <QDockWidget>

#include "ui_fourdsoundpanel.h"


class FourDSoundPanel : public QDockWidget
{
	Q_OBJECT
public:
	explicit FourDSoundPanel(QWidget *parent = 0);

private:
	Ui::FourDSoundPanelUI ui;
};

