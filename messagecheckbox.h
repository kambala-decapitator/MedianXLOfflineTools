#ifndef MESSAGECHECKBOX_H
#define MESSAGECHECKBOX_H

#include <QtGlobal>

#ifdef Q_WS_MACX
//@class NSAlert;
class QWidget;

class MessageCheckBox
{
public:
    MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent = 0);
    virtual ~MessageCheckBox();

    void setChecked(bool checked);
    bool isChecked();

public:
    int exec();

private:
//    NSAlert *_alert;
};
#else
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
#endif // Q_WS_MACX

#endif // MESSAGECHECKBOX_H
