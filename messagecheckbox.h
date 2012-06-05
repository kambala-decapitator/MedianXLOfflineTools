#ifndef MESSAGECHECKBOX_H
#define MESSAGECHECKBOX_H

class MessageCheckBoxPrivateBase
{
public:
    virtual ~MessageCheckBoxPrivateBase() {}

    virtual void setChecked(bool checked) = 0;
    virtual bool isChecked() = 0;

    virtual int exec() = 0;
};


class QString;
class QWidget;

class MessageCheckBox
{
public:
    MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent = 0);
    virtual ~MessageCheckBox() { delete d; }

    void setChecked(bool checked) { d->setChecked(checked); }
    bool isChecked() { return d->isChecked(); }

    int exec() { return d->exec(); }

private:
    MessageCheckBoxPrivateBase *d;
};

#endif // MESSAGECHECKBOX_H
