#include "fileselector.h"

FileSelector::FileSelector() : QWidget()
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

void FileSelector::setFileFilter(const QString& filter)
{ mFileFilter = filter; }

void FileSelector::setFilename(const QString& filename)
{
    mLineEdit.setText(filename);
}

const QString FileSelector::filename()
{
    return mLineEdit.text().trimmed();
}

void FileSelector::onEditingFinished()
{
    filenameChanged(filename());
}

void FileSelector::onBrowseButtonClicked()
{
    QString dir = filename().isEmpty() ? "." : QFileInfo(filename()).path();

    auto f = QFileDialog::getOpenFileName(this, "Select File", dir, mFileFilter);
    if (f.isEmpty())
        return;

    mLineEdit.setText(f);
    filenameChanged(filename());
}
