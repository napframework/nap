#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>

class FileSelector : public QWidget {
Q_OBJECT
public:
    FileSelector() : QWidget()
    {
        setLayout(&mLayout);
        mLayout.setContentsMargins(0, 0, 0, 0);
        mLayout.setSpacing(2);

        mLayout.addWidget(&mLineEdit);
        mLayout.addWidget(&mBrowseButton);

        mBrowseButton.setText("...");

        connect(&mLineEdit, &QLineEdit::editingFinished, this, &FileSelector::onEditingFinished);
        connect(&mBrowseButton, &QToolButton::clicked, this, &FileSelector::onBrowseButtonClicked);
    }

    void setFileFilter(const QString& filter)
    { mFileFilter = filter; }

    void setFilename(const QString& filename)
    {
        mLineEdit.setText(filename);
    }

    const QString filename()
    {
        return mLineEdit.text().trimmed();
    }

Q_SIGNALS:

    void filenameChanged(const QString& filename);

private:
    void onEditingFinished()
    {
        filenameChanged(filename());
    }

    void onBrowseButtonClicked()
    {
        QString dir = filename().isEmpty() ? "." : QFileInfo(filename()).path();

        auto f = QFileDialog::getOpenFileName(this, "Select File", dir, mFileFilter);
        if (f.isEmpty())
            return;

        mLineEdit.setText(f);
        filenameChanged(filename());
    }

    QHBoxLayout mLayout;
    QLineEdit mLineEdit;
    QToolButton mBrowseButton;
    QString mFileFilter;
};