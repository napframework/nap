#include <QtWidgets/QAbstractButton>
#include "fileselector.h"



napkin::FileSelector::FileSelector() : QWidget()
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

void napkin::FileSelector::setFileFilter(const QString& filter)
{
	mFileFilter = filter;
}

void napkin::FileSelector::setFilename(const QString& filename)
{
	mLineEdit.setText(filename);
}

const QString napkin::FileSelector::getFilename()
{
	return mLineEdit.text().trimmed();
}

void napkin::FileSelector::onEditingFinished()
{
	filenameChanged(getFilename());
}

void napkin::FileSelector::onBrowseButtonClicked()
{
	QString dir = getFilename().isEmpty() ? "." : QFileInfo(getFilename()).path();

	QString f = QFileDialog::getOpenFileName(this, "Select File", dir, mFileFilter);
	if (f.isEmpty())
		return;

	mLineEdit.setText(f);
	filenameChanged(getFilename());
}
