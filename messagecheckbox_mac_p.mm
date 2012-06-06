#include "messagecheckbox.h"

#include <QCoreApplication>

#import <AppKit/NSAlert.h>
#import <AppKit/NSButton.h>


class MessageCheckBoxImpl
{
public:
    MessageCheckBoxImpl(const QString &text, const QString &checkboxText, QWidget *parent = 0) : alert([[NSAlert alloc] init])
    {
        Q_UNUSED(parent);

        [alert setAlertStyle:NSCriticalAlertStyle];
        [alert setMessageText:nsstringFromQstring(text)];
        [alert setShowsHelp:NO];
        [alert setShowsSuppressionButton:YES];
        [[alert suppressionButton] setTitle:nsstringFromQstring(checkboxText)];
        // dirty hack to remove ampersands from strings
        [alert addButtonWithTitle:nsstringFromQstring(qApp->translate("QDialogButtonBox", "&Yes").remove('&'))]; // returns 1000
        [alert addButtonWithTitle:nsstringFromQstring(qApp->translate("QDialogButtonBox", "&No" ).remove('&'))]; // returns 1001
    }

    virtual ~MessageCheckBoxImpl() { [alert release]; }

    static NSString *nsstringFromQstring(const QString &s) { return [NSString stringWithCharacters:(const unichar *)s.unicode() length:s.length()]; }

    NSAlert *alert;
};


// MessageCheckBox implementation

MessageCheckBox::MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent /*= 0*/) : _impl(new MessageCheckBoxImpl(text, checkboxText, parent)) {}
MessageCheckBox::~MessageCheckBox() { delete _impl; }

void MessageCheckBox::setChecked(bool checked) { [[_impl->alert suppressionButton] setState:(checked ? NSOnState : NSOffState)]; }
bool MessageCheckBox::isChecked() const { return [[_impl->alert suppressionButton] state] == NSOnState; }

int MessageCheckBox::exec() { return !([_impl->alert runModal] - 1000); }
