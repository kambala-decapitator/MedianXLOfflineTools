#include "skillplandialog.h"
#include "resourcepathmanager.hpp"

#include <QClipboard>


static const QString defaultModVersionReadable("2012 v005"), defaultModVersionPlanner("2012005");

QString SkillplanDialog::_modVersionReadable;
QString SkillplanDialog::_modVersionPlanner;

void SkillplanDialog::loadModVersion()
{
    QFile f(ResourcePathManager::dataPathForFileName("version.txt"));
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!f.atEnd())
        {
            QList<QByteArray> lineData = f.readLine().split('\t');
            if (lineData.size() > 1)
            {
                QByteArray text = lineData.at(1).trimmed();
                if (lineData.at(0) == "readable")
                    _modVersionReadable = text;
                else if (lineData.at(0) == "planner")
                    _modVersionPlanner = text;
            }
        }
    }
    if (_modVersionReadable.isEmpty())
        _modVersionReadable = defaultModVersionReadable;
    if (_modVersionPlanner.isEmpty())
        _modVersionPlanner = defaultModVersionPlanner;
}

SkillplanDialog::SkillplanDialog(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);
    setWindowModality(Qt::WindowModal);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui.copyHtmlButton, SIGNAL(clicked()), SLOT(copyHtml()));
    connect(ui.copyBbcodeButton, SIGNAL(clicked()), SLOT(copyBbcode()));

    setFixedSize(sizeHint());
}

void SkillplanDialog::copyHtml()
{
    qApp->clipboard()->setText(ui.htmlLinkLineEdit->text());
}

void SkillplanDialog::copyBbcode()
{
    qApp->clipboard()->setText(ui.bbcodeLinkLineEdit->text());
}
