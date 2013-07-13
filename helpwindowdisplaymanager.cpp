#include "helpwindowdisplaymanager.h"

#include <QWidget>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QDesktopServices>

#include <QPointer>
#include <QUrl>


class HelpWindow : public QWidget
{
    Q_OBJECT

public:
    HelpWindow(const QString &windowTitle, const QString &helpText, QWidget *parent = 0) : QWidget(parent), _textBrowser(new QTextBrowser(this))
    {
        setWindowFlags(Qt::Dialog);
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setAttribute(Qt::WA_DeleteOnClose);
        setAttribute(Qt::WA_QuitOnClose, false);
        setWindowTitle(windowTitle);

        _textBrowser->setOpenExternalLinks(true);
        _textBrowser->setHtml(helpText);
        connect(_textBrowser, SIGNAL(anchorClicked(QUrl)), SLOT(openLocalUrl(QUrl)));

        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
        connect(buttonBox, SIGNAL(rejected()), SLOT(close()));

        QVBoxLayout *vbox = new QVBoxLayout(this);
        vbox->addWidget(_textBrowser);
        vbox->addWidget(buttonBox);

        resize(400, 400);
    }

    virtual ~HelpWindow() {}

private slots:
    void openLocalUrl(const QUrl &url)
    {
        _textBrowser->setSource(QUrl()); // don't open blank page in the browser
        if (url.scheme() == QLatin1String("file"))
            QDesktopServices::openUrl(url);
    }

private:
    QTextBrowser *_textBrowser;
};

#include "helpwindowdisplaymanager.moc"


struct HelpWindowDisplayManagerImpl
{
    QPointer<HelpWindow> helpWindow;
};


HelpWindowDisplayManager::~HelpWindowDisplayManager()
{
    if (_impl)
    {
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
