#include "fileselector.h"

#include <QAbstractButton>


napqt::FileSelector::FileSelector() : QWidget()
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

void napqt::FileSelector::setFileFilter(const QString& filter)
{
	mFileFilter = filter;
}

void napqt::FileSelector::setFilename(const QString& filename)
{
	mLineEdit.setText(filename);
}

const QString napqt::FileSelector::getFilename()
{
	return mLineEdit.text().trimmed();
}

void napqt::FileSelector::onEditingFinished()
{
	filenameChanged(getFilename());
}

void napqt::FileSelector::onBrowseButtonClicked()
{
	QString dir = getFilename().isEmpty() ? "." : QFileInfo(getFilename()).path();

	QString f = QFileDialog::getOpenFileName(this, "Select File", dir, mFileFilter);
	if (f.isEmpty())
		return;

	mLineEdit.setText(f);
	filenameChanged(getFilename());
}
