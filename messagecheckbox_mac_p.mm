#include "messagecheckbox.h"

// #include <QStringList>
#include <QCoreApplication>

#import <AppKit/NSAlert.h>
#import <AppKit/NSButton.h>

#define NSSTRING_FROM_QSTRING(s) [NSString stringWithCharacters:(const unichar *)s.unicode() length:s.length()]


class MacMessageCheckBoxPrivate : public MessageCheckBoxPrivateBase
{
public:
    MacMessageCheckBoxPrivate(const QString &text, const QString &checkboxText, QWidget *parent = 0)
    {
        Q_UNUSED(parent);

        _alert = [[NSAlert alloc] init];
        [_alert setAlertStyle:NSCriticalAlertStyle];
        [_alert setMessageText:NSSTRING_FROM_QSTRING(text)];
        [_alert setShowsHelp:NO];
        [_alert setShowsSuppressionButton:YES];
        [[_alert suppressionButton] setTitle:NSSTRING_FROM_QSTRING(checkboxText)];
        // dirty hack to remove ampersands from strings
        [_alert addButtonWithTitle:NSSTRING_FROM_QSTRING(qApp->translate("QDialogButtonBox", "&Yes").remove('&'))]; // returns 1000
        [_alert addButtonWithTitle:NSSTRING_FROM_QSTRING(qApp->translate("QDialogButtonBox", "&No" ).remove('&'))]; // returns 1001
    }

    virtual ~MacMessageCheckBoxPrivate() { [_alert release]; }

    virtual void setChecked(bool checked) { [[_alert suppressionButton] setState:(checked ? NSOnState : NSOffState)]; }
    virtual bool isChecked() { return [[_alert suppressionButton] state] == NSOnState; }

public slots:
    virtual int exec() { return !([_alert runModal] - 1000); }

private:
    NSAlert *_alert;
};


MessageCheckBox::MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent /*= 0*/) : d(new MacMessageCheckBoxPrivate(text, checkboxText, parent)) {}
