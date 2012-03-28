#ifndef MESSAGECHECKBOX_H
#define MESSAGECHECKBOX_H

#include <QDialog>


class QLabel;
class QCheckBox;
class QDialogButtonBox;

class MessageCheckBox : public QDialog
{
    Q_OBJECT

public:
    MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent = 0);

    void setChecked(bool checked);
    bool isChecked();

private:
    QLabel *_text;
    QCheckBox *_checkBox;
    QDialogButtonBox *_buttonBox;
};

#endif // MESSAGECHECKBOX_H
