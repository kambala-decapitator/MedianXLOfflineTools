#ifndef MESSAGECHECKBOX_H
#define MESSAGECHECKBOX_H

class QString;
class QWidget;
class MessageCheckBoxImpl;

class MessageCheckBox
{
public:
    MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent = 0);
    virtual ~MessageCheckBox();

    void setChecked(bool checked);
    bool isChecked() const;

    int exec();

private:
    MessageCheckBoxImpl *_impl;
};

#endif // MESSAGECHECKBOX_H
