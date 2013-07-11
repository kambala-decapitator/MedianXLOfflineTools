#include "helpwindowdisplaymanager.h"

#include <QWidget>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPointer>


class HelpWindow : public QWidget
{
public:
    HelpWindow(const QString &windowTitle, const QString &helpText, QWidget *parent = 0) : QWidget(parent)
    {
        setWindowFlags(Qt::Dialog);
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setAttribute(Qt::WA_DeleteOnClose);
        setAttribute(Qt::WA_QuitOnClose, false);
        setWindowTitle(windowTitle);

        QTextBrowser *textBrowser = new QTextBrowser(this);
        textBrowser->setOpenExternalLinks(true);
        textBrowser->setHtml(helpText);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
        connect(buttonBox, SIGNAL(rejected()), SLOT(close()));

        QVBoxLayout *vbox = new QVBoxLayout(this);
        vbox->addWidget(textBrowser);
        vbox->addWidget(buttonBox);

        resize(400, 400);
    }

    virtual ~HelpWindow() {}
};


struct HelpWindowDisplayManagerImpl
{
    QPointer<HelpWindow> helpWindow;
};


HelpWindowDisplayManager::~HelpWindowDisplayManager()
{
    if (_impl)
    {
        if (_impl->helpWindow)
            delete _impl->helpWindow;
        delete _impl;
    }
}

void HelpWindowDisplayManager::showHelp()
{
    if (!_impl)
        _impl = new HelpWindowDisplayManagerImpl;

    if (!_impl->helpWindow)
    {
        // widget without parent is located under the modal widget, so set parent to modal widget
        QWidget *parentWidget = qobject_cast<QWidget *>(parent());
        _impl->helpWindow = new HelpWindow(_helpWindowTitle, _helpWindowText, parentWidget && parentWidget->isModal() ? parentWidget : 0);
        _impl->helpWindow->show();
    }
    else
    {
        _impl->helpWindow->activateWindow();
        _impl->helpWindow->raise();
    }
}

void HelpWindowDisplayManager::closeHelp()
{
    if (_impl && _impl->helpWindow)
        _impl->helpWindow->close();
}
