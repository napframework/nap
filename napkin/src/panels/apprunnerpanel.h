#pragma once

#include <QWidget>
#include <QPushButton>
#include <QProcess>
#include <QHBoxLayout>
#include <generic/fileselector.h>
#include <QSettings>
#include <QThread>
#include <generic/utility.h>
#include "appcontext.h"

#define LAST_CORE_APP "lastCoreApp"

#define TXT_START "Start"
#define TXT_STOP "Stop"

class AppRunnerPanel : public QWidget {
Q_OBJECT
public:
    AppRunnerPanel();
    ~AppRunnerPanel();
protected:
    void showEvent(QShowEvent* event) override;

private:
    void onAppChanged(const QString& filename);
    void onStartApp();
    void onStopApp();
    void onReadOut();
    void onReadErr();
    void onAppStarted();
    void onAppError(QProcess::ProcessError error);
    void onAppState(QProcess::ProcessState state);
    void onAppFinished(int exitCode);

    FileSelector mFileSelector;
    QHBoxLayout mLayout;
    QPushButton mStartButton;
    QPushButton mStopButton;
    QProcess mProcess;
};