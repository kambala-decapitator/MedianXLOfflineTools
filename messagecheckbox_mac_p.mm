#include "messagecheckbox.h"

#import "machelpers.h"

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
        [alert setMessageText:NSStringFromQString(text)];
        [alert setShowsHelp:NO];
        [alert setShowsSuppressionButton:YES];
        [[alert suppressionButton] setTitle:NSStringFromQString(checkboxText)];
        // dirty hack to remove ampersands from strings
        [alert addButtonWithTitle:NSStringFromQString(qApp->translate("QDialogButtonBox", "&Yes").remove('&'))]; // returns 1000
        [alert addButtonWithTitle:NSStringFromQString(qApp->translate("QDialogButtonBox", "&No" ).remove('&'))]; // returns 1001
    }

    virtual ~MessageCheckBoxImpl() { [alert release]; }

    NSAlert *alert;
};


// MessageCheckBox implementation

MessageCheckBox::MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent /*= 0*/) : _impl(new MessageCheckBoxImpl(text, checkboxText, parent)) {}
MessageCheckBox::~MessageCheckBox() { delete _impl; }

void MessageCheckBox::setChecked(bool checked) { [[_impl->alert suppressionButton] setState:(checked ? NSOnState : NSOffState)]; }
bool MessageCheckBox::isChecked() const { return [[_impl->alert suppressionButton] state] == NSOnState; }

int MessageCheckBox::exec() { return !([_impl->alert runModal] - 1000); }
