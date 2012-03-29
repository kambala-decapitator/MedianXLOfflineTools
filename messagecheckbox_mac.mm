#include "messagecheckbox.h"

#include <QStringList>
#include <QCoreApplication>

#import <AppKit/NSAlert.h>
#import <AppKit/NSButton.h>

#define NSSTRING_FROM_QSTRING(s) [NSString stringWithCharacters:(const unichar *)s.unicode() length:s.length()]


NSAlert *_alert = nil;

MessageCheckBox::MessageCheckBox(const QString &text, const QString &checkboxText, QWidget *parent)
{
    Q_UNUSED(parent);

    _alert = [[NSAlert alloc] init];
    [_alert setAlertStyle:NSCriticalAlertStyle];
    [_alert setMessageText:NSSTRING_FROM_QSTRING(text)];
    // dirty hack to remove ampersands from strings
    [_alert addButtonWithTitle:NSSTRING_FROM_QSTRING(qApp->translate("QDialogButtonBox", "&Yes").remove('&'))]; // returns 1000
    [_alert addButtonWithTitle:NSSTRING_FROM_QSTRING(qApp->translate("QDialogButtonBox", "&No").remove('&'))];  // returns 1001
    [_alert setShowsHelp:NO];
    [_alert setShowsSuppressionButton:YES];
    [[_alert suppressionButton] setTitle:NSSTRING_FROM_QSTRING(checkboxText)];
}

MessageCheckBox::~MessageCheckBox()
{
    [_alert release];
}

void MessageCheckBox::setChecked(bool checked)
{
    [[_alert suppressionButton] setState:(checked ? NSOnState : NSOffState)];
}

bool MessageCheckBox::isChecked()
{
    return [[_alert suppressionButton] state] == NSOnState;
}

int MessageCheckBox::exec()
{
    return !([_alert runModal] - 1000);
}
