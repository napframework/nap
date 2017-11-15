#pragma once

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/attributeobject.h>



class Action : public QAction
{
public:
    Action(const QString& text);

	void setCategory(const QString& cat) { mCategory = cat; }
	const QString& category() const { return mCategory; }

	virtual void perform() const = 0;
	virtual bool canActOn(const nap::Object& obj) const { return false; }
	virtual bool isAvailable() { return true; }
	virtual QString iconName() { return QString(); }

private:
	QString mCategory;
};


class ActionStore
{
public:
	ActionStore();

	const QList<Action*>& actions() { return mActions; }
	void init();
	bool perform(const QString& actionName);
	QAction* action(const QString& name);
	QList<QAction*> actionsFor(const nap::Object& object);
	void updateStates();

private:
	QList<Action*> mActions;
};
