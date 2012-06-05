#include "messagecheckbox.h"

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>


class MessageCheckBoxPrivate : public QDialog, public MessageCheckBoxPrivateBase
{
    Q_OBJECT

public:
    MessageCheckBoxPrivate(const QString &text, const QString &checkboxText, QWidget *parent = 0) : QDialog(parent), textLabel(new QLabel(text, this)),
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

    virtual ~MessageCheckBoxPrivate() {}

    virtual void setChecked(bool checked) { checkBox->setChecked(checked); }
    virtual bool isChecked() { return checkBox->isChecked(); }

public slots:
    virtual int exec() { return QDialog::exec(); }

private:
    QLabel *textLabel;
    QCheckBox *checkBox;
    QDialogButtonBox *buttonBox;
};

#include "messagecheckbox_p.moc"


MessageCheckBox::MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent /*= 0*/) : d(new MessageCheckBoxPrivate(text, checkboxText, parent)) {}
