#include    "skillplandialog.h"
#include "ui_skillplandialog.h"
#include "resourcepathmanager.hpp"
#include "characterinfo.hpp"
#include "itemdatabase.h"

#include <QClipboard>

#include <QSettings>


// TODO: check current mod version
static const QString kDefaultModVersionReadable("2012 v005"), kDefaultModVersionPlanner("2012005");
static const QString kBaseUrl("http://www.authmann.de/d2/mxl/skillpointplanner/?v=%1&class=%2&lvl=%3&bonus=%4&skills=%5&mg=%6&sos=%8&name=%7");

QString SkillplanDialog::_modVersionReadable;
QString SkillplanDialog::_modVersionPlanner;

bool SkillplanDialog::didModVersionChange()
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
        _modVersionReadable = kDefaultModVersionReadable;
    if (_modVersionPlanner.isEmpty())
        _modVersionPlanner = kDefaultModVersionPlanner;

    QSettings settings;
    QVariant lastUsedModVersion = settings.value("lastUsedModVersion");
    if (lastUsedModVersion.toString() != _modVersionReadable)
    {
        settings.setValue("lastUsedModVersion", _modVersionReadable);
        if (lastUsedModVersion.isValid())
            return true;
    }
    return false;
}


SkillplanDialog::SkillplanDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SkillplanDialog)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    ui->copyHtmlButton->setObjectName("html");
    ui->copyBbcodeButton->setObjectName("bbcode");

    connect(ui->copyHtmlButton,   SIGNAL(clicked()), SLOT(copyLinkWithTags()));
    connect(ui->copyBbcodeButton, SIGNAL(clicked()), SLOT(copyLinkWithTags()));
    connect(ui->buttonBox, SIGNAL(helpRequested()), SLOT(showHelp()));

    foreach (QCheckBox *checkBox, QList<QCheckBox *>() << ui->skillQuestsCheckBox << ui->charmsCheckBox << ui->minigamesCheckBox << ui->signetsCheckBox << ui->itemsCheckBox)
        connect(checkBox, SIGNAL(clicked()), SLOT(constructUrls()));

    loadSettings();
    constructUrls();
}

SkillplanDialog::~SkillplanDialog()
{
    delete ui;
}


void SkillplanDialog::done(int r)
{
    saveSettings();
    QDialog::done(r);
}


void SkillplanDialog::copyLinkWithTags()
{
    qApp->clipboard()->setText((sender()->objectName() == QLatin1String("html") ? ui->htmlLinkLineEdit : ui->bbcodeLinkLineEdit)->text());
}

void SkillplanDialog::constructUrls()
{
    const CharacterInfo &charInfo = CharacterInfo::instance();
    QString skillQuests;
    if (ui->skillQuestsCheckBox->isChecked())
    {
        QList<QList<bool> > skillQuestsList = QList<QList<bool> >() << charInfo.questsInfo.denOfEvil << charInfo.questsInfo.radament << charInfo.questsInfo.izual;
        foreach (const QList<bool> &list, skillQuestsList)
            foreach (bool isCompleted, list)
                skillQuests += QString::number(isCompleted);
    }
    else
        skillQuests = "111111111";

    QString maxSkillLevelCharms;
    ItemInfo *classCharm = itemOr0(charInfo, isClassCharm);
    if (ui->charmsCheckBox->isChecked())
    {
        maxSkillLevelCharms = classCharm ? QString("%1%2").arg(classCharm->props.contains(Enums::ClassCharmChallenges::Ennead)).arg(classCharm->props.contains(Enums::ClassCharmChallenges::BlackRoad)) : "00";
        maxSkillLevelCharms += hasItemBoolString(charInfo, isMoonOfSpider);
        maxSkillLevelCharms += hasItemBoolString(charInfo, isLegacyOfBlood);
        maxSkillLevelCharms += classCharm ? QString::number(classCharm->props.contains(Enums::ClassCharmChallenges::Bremmtown)) : "0";
        ItemInfo *cfm = itemOr0(charInfo, isCrystallineFlameMedallion);
        maxSkillLevelCharms += QString::number(cfm && cfm->props.contains(Enums::ItemProperties::MaxSlvlIncrease));
    }
    else
        maxSkillLevelCharms = "111111";

    QString maxSkillLevelItems = ui->itemsCheckBox->isChecked() ? hasItemBoolString(charInfo, isDrekavacInGear) + hasItemBoolString(charInfo, isVeneficaInGear) : "11";

    QString minigames;
    if (ui->minigamesCheckBox->isChecked())
    {
        // "1" is Windows in Hell - it was removed in Omega, and bonus transferred to the class charm itself
        minigames = !classCharm ? "0000" : QString("%1%2%3%4").arg(classCharm->props.contains(Enums::ClassCharmChallenges::Countess))
                                                              .arg(classCharm->props.contains(Enums::ClassCharmChallenges::Crowned)).arg(1)
                                                              .arg(classCharm->props.contains(Enums::ClassCharmChallenges::MirrorMirror));
    }
    else
        minigames = "1111";

    QString skillStr;
    foreach (quint8 skillValue, charInfo.basicInfo.skillsReadable)
        skillStr += QString("%1_").arg(skillValue);

    Enums::ClassName::ClassNameEnum classCode = charInfo.basicInfo.classCode;
    quint8 classCodePlanner;
    switch (classCode)
    {
    case Enums::ClassName::Amazon:
        classCodePlanner = 1;
        break;
    case Enums::ClassName::Assassin:
        classCodePlanner = 2;
        break;
    case Enums::ClassName::Barbarian:
        classCodePlanner = 3;
        break;
    case Enums::ClassName::Druid:
        classCodePlanner = 4;
        break;
    case Enums::ClassName::Necromancer:
        classCodePlanner = 5;
        break;
    case Enums::ClassName::Paladin:
        classCodePlanner = 6;
        break;
    case Enums::ClassName::Sorceress:
        classCodePlanner = 7;
        break;
    default: // unreachable
        classCodePlanner = 0;
        break;
    }

    QString url = kBaseUrl.arg(modVersionPlanner()).arg(classCodePlanner).arg(charInfo.basicInfo.level).arg(skillQuests + maxSkillLevelCharms + maxSkillLevelItems, skillStr, minigames, charInfo.basicInfo.newName)
                          .arg(ui->signetsCheckBox->isChecked() ? charInfo.valueOfStatistic(Enums::CharacterStats::SignetsOfSkillEaten) : Enums::CharacterStats::SignetsOfSkillMax);
    QString linkText = QString("%1 %2 (%3, %4)").arg(tr("Skillplan"), charInfo.basicInfo.newName, Enums::ClassName::classes().at(classCode), modVersionReadable());
    QString htmlText = QString("<a href = \"%1\">%2</a>").arg(url, linkText);
    QString bbcodeText = QString("[url=%1]%2[/url]").arg(url, linkText);

    ui->linkLabel->setText(htmlText);
    ui->htmlLinkLineEdit->setText(htmlText);
    ui->bbcodeLinkLineEdit->setText(bbcodeText);
}

void SkillplanDialog::showHelp()
{
    INFO_BOX(tr("If you check a checkbox, then corresponding value is taken from your character. If you don't, then maximum possible value is used.\n\n"
                "To copy simple link: right-click the blue-highlighted link and click on the menu command."));
}


void SkillplanDialog::loadSettings()
{
    QSettings settings;
    settings.beginGroup("skillplanDialog");

    ui->skillQuestsCheckBox->setChecked(settings.value("skillQuests").toBool());
    ui->charmsCheckBox->setChecked(settings.value("charms").toBool());
    ui->minigamesCheckBox->setChecked(settings.value("minigames").toBool());
    ui->signetsCheckBox->setChecked(settings.value("signets").toBool());
    ui->itemsCheckBox->setChecked(settings.value("items").toBool());

    settings.endGroup();
}

void SkillplanDialog::saveSettings()
{
    QSettings settings;
    settings.beginGroup("skillplanDialog");

    settings.setValue("skillQuests", ui->skillQuestsCheckBox->isChecked());
    settings.setValue("charms", ui->charmsCheckBox->isChecked());
    settings.setValue("minigames", ui->minigamesCheckBox->isChecked());
    settings.setValue("signets", ui->signetsCheckBox->isChecked());
    settings.setValue("items", ui->itemsCheckBox->isChecked());

    settings.endGroup();
}

ItemInfo *SkillplanDialog::itemOr0(const CharacterInfo &charInfo, bool(*predicate)(ItemInfo *))
{
    ItemsList::const_iterator iter = std::find_if(charInfo.items.character.constBegin(), charInfo.items.character.constEnd(), predicate);
    return iter != charInfo.items.character.constEnd() ? *iter : 0;
}
