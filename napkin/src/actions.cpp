#include "actions.h"

using namespace napkin;

Action::Action() : QAction() { connect(this, &QAction::triggered, this, &Action::perform); }

NewFileAction::NewFileAction()
{
	setText("New");
	setShortcut(QKeySequence::New);
}

void NewFileAction::perform() { AppContext::get().newFile(); }


OpenFileAction::OpenFileAction()
{
	setText("Open...");
	setShortcut(QKeySequence::Open);
}

void OpenFileAction::perform()
{
	auto lastFilename = AppContext::get().lastOpenedFilename();
	QString filename = QFileDialog::getOpenFileName(QApplication::topLevelWidgets()[0], "Open NAP Data File",
													lastFilename, JSON_FILE_FILTER);
	if (filename.isNull())
		return;

	AppContext::get().loadFile(filename);
}

SaveFileAction::SaveFileAction()
{
	setText("Save");
	setShortcut(QKeySequence::Save);
}

void SaveFileAction::perform()
{
	if (AppContext::get().currentFilename().isNull())
	{
		SaveFileAsAction().trigger();
		return;
	}
	AppContext::get().saveFile();
}

SaveFileAsAction::SaveFileAsAction()
{
	setText("Save as...");
	setShortcut(QKeySequence::SaveAs);
}

void SaveFileAsAction::perform()
{
	auto& ctx = AppContext::get();
	auto prevFilename = ctx.currentFilename();
	if (prevFilename.isNull())
		prevFilename = ctx.lastOpenedFilename();

	QString filename = QFileDialog::getSaveFileName(QApplication::topLevelWidgets()[0], "Save NAP Data File",
													prevFilename, JSON_FILE_FILTER);

	if (filename.isNull())
		return;

	ctx.saveFileAs(filename);
}
