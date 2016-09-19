#include "actionstore.h"
#include "actions.h"
#include "commands.h"


Action::Action(const QString& text) : QAction(text, QCoreApplication::instance())
{
	connect(this, &QAction::triggered, this, &Action::perform);
}


ActionStore::ActionStore()
{
	mActions << new NewAction();
	mActions << new OpenAction();
	mActions << new SaveAction();
	mActions << new SaveAsAction();

	mActions << new QuitAction();

	mActions << new CreateEntityAction();
	mActions << new CreateComponentAction();
	mActions << new CreateOperatorAction();
	mActions << new AddAttributeAction();

	mActions << new CutAction();
	mActions << new CopyAction();
	mActions << new DuplicateAction();
	mActions << new PasteAction();
	mActions << new DeleteAction();
}

void ActionStore::init()
{
	for (auto action : mActions) {
		action->setIconVisibleInMenu(true);
		if (!action->icon().isNull()) action->setIcon(*AppContext::get().iconStore().get(action->iconName()));
	}
}
bool ActionStore::perform(const QString& actionName)
{
	QAction* a = action(actionName);
	if (!a) {
		nap::Logger::fatal("No action '%s' registered.", actionName.toStdString().c_str());
		return false;
	}
	a->trigger();
	return true;
}
QAction* ActionStore::action(const QString& name)
{
	for (auto action : mActions) {
		if (action->text() == name) return action;
	}
	return nullptr;
}
QList<QAction*> ActionStore::actionsFor(const nap::Object& object)
{
	QList<QAction*> actions;
	for (auto action : mActions)
		if (action->canActOn(object)) actions << action;
	return actions;
}
void ActionStore::updateStates()
{
	for (auto action : mActions)
		action->setEnabled(action->isAvailable());
}
