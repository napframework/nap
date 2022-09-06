/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "fileselector.h"
#include <QAbstractButton>

using namespace nap::qt;

FileSelector::FileSelector(QWidget* parent) : QWidget(parent)
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.setSpacing(2);

	mLayout.addWidget(&mLineEdit);
	mLayout.addWidget(&mBrowseButton);

	mBrowseButton.setText("...");

	connect(&mLineEdit, &QLineEdit::editingFinished, this, &FileSelector::onEditingFinished);
	connect(&mBrowseButton, &QPushButton::clicked, this, &FileSelector::onBrowseButtonClicked);
}

void FileSelector::setFileFilter(const QString& filter)
{
	mFileFilter = filter;
}

void FileSelector::setFilename(const QString& filename)
{
	mLineEdit.setText(filename);
}

const QString FileSelector::getFilename()
{
	return mLineEdit.text().trimmed();
}

void FileSelector::onEditingFinished()
{
	filenameChanged(getFilename());
}

void FileSelector::onBrowseButtonClicked()
{
	QString dir = getFilename().isEmpty() ? "." : QFileInfo(getFilename()).path();

#ifdef __linux__
    QString f = QFileDialog::getOpenFileName(this, "Select File", dir, mFileFilter, nullptr, QFileDialog::DontUseNativeDialog);
#else
    QString f = QFileDialog::getOpenFileName(this, "Select File", dir, mFileFilter, nullptr);
#endif
	if (f.isEmpty())
		return;

	mLineEdit.setText(f);
	filenameChanged(getFilename());
}
