#include "messagecheckbox.h"

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>


class MessageCheckBoxImpl : public QDialog
{
    Q_OBJECT

public:
    MessageCheckBoxImpl(const QString &text, const QString &checkboxText, QWidget *parent = 0) : QDialog(parent), textLabel(new QLabel(text, this)),
        checkBox(new QCheckBox(checkboxText, this)), buttonBox(new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, Qt::Horizontal, this))
    {
        QStyle *style_ = style();
        int iconSize = style_->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, this);
        QPixmap iconPixmap = style_->standardIcon(QStyle::SP_MessageBoxWarning, 0, this).pixmap(iconSize, iconSize);
        QLabel *iconLabel = new QLabel(this);
        iconLabel->setPixmap(iconPixmap);

        QGridLayout *grid = new QGridLayout;
        grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
        grid->addWidget(textLabel, 0, 1);
        grid->addWidget(checkBox, 1, 1);
        grid->setVerticalSpacing(20);
        grid->setHorizontalSpacing(20);
        grid->setContentsMargins(0, 0, 0, 10);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addLayout(grid);
        layout->addWidget(buttonBox);

        connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

        setFixedSize(sizeHint());
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setWindowTitle(qApp->applicationName());
    }

    QLabel *textLabel;
    QCheckBox *checkBox;
    QDialogButtonBox *buttonBox;
};

#include "messagecheckbox_p.moc"


// MessageCheckBox implementation

MessageCheckBox::MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent /*= 0*/) : _impl(new MessageCheckBoxImpl(text, checkboxText, parent)) {}
MessageCheckBox::~MessageCheckBox() {}

void MessageCheckBox::setChecked(bool checked) { _impl->checkBox->setChecked(checked); }
bool MessageCheckBox::isChecked() const { return _impl->checkBox->isChecked(); }

int MessageCheckBox::exec() { return _impl->exec(); }
