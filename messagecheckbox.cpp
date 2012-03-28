#include "messagecheckbox.h"

#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>


MessageCheckBox::MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent) : QDialog(parent)
{
    _text = new QLabel(text, this);
    _checkBox = new QCheckBox(checkboxText, this);
    _buttonBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, Qt::Horizontal, this);

    QStyle *style_ = style();
    int iconSize = style_->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this);
    QPixmap iconPixmap = style_->standardIcon(QStyle::SP_MessageBoxWarning, 0, this).pixmap(iconSize, iconSize);
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(iconPixmap);

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
    grid->addWidget(_text, 0, 1);
    grid->addWidget(_checkBox, 1, 1);
    grid->setVerticalSpacing(20);
    grid->setHorizontalSpacing(20);
    grid->setContentsMargins(0, 0, 0, 10);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(grid);
    //layout->addWidget(_text);
    //layout->addWidget(_checkBox);
    layout->addWidget(_buttonBox);

    connect(_buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(_buttonBox, SIGNAL(rejected()), SLOT(reject()));

    setFixedSize(sizeHint());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(qApp->applicationName());
}

void MessageCheckBox::setChecked(bool checked)
{
    _checkBox->setChecked(checked);
}

bool MessageCheckBox::isChecked()
{
    return _checkBox->isChecked();
}
