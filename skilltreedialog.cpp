#include "skilltreedialog.h"
#include "characterinfo.hpp"
#include "itemdatabase.h"
#include "resourcepathmanager.hpp"
#include "helpers.h"

#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include <QSettings>


class SkillWidget : public QWidget
{
public:
    SkillWidget(QWidget *parent = 0) : QWidget(parent), _skillNameLabel(new QLabel(this)), _skillImageLabel(new QLabel(this)), _skillPointsLabel(new QLabel(this))
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(_skillNameLabel,   0, Qt::AlignHCenter);
        layout->addWidget(_skillImageLabel,  0, Qt::AlignHCenter);
        layout->addWidget(_skillPointsLabel, 0, Qt::AlignHCenter);
    }
    virtual ~SkillWidget() {}

    void setSkillName(const QString &name) { _skillNameLabel->setText(name); }

    void setSkillImageForClassWithId(int classCode, int imageId)
    {
        QString path = ResourcePathManager::pathForSkillImage(classCode, imageId);
        if (!QFile::exists(path))
            path = ResourcePathManager::pathForSkillImage(classCode, --imageId);
        _skillImageLabel->setPixmap(QPixmap(path));
    }

    void setSkillPoints(quint8 basePoints, quint8 addPoints)
    {
        quint8 maxClvl = static_cast<quint8>(isUltimative() ? Enums::CharacterStats::MaxNonHardenedLevel : Enums::CharacterStats::MaxLevel);
        quint8 actualBasePoints  = qMin(maxClvl, basePoints);
        quint8 actualTotalPoints = qMin(maxClvl, static_cast<quint8>(basePoints + addPoints));

        _skillPointsLabel->setText(addPoints ? QString("%1 (%2)").arg(actualBasePoints).arg(actualTotalPoints) : QString::number(actualBasePoints));
        if (!basePoints)
            setDisabled(true);
    }

private:
    QLabel *_skillNameLabel, *_skillImageLabel, *_skillPointsLabel;
};


SkillTreeDialog::SkillTreeDialog(QWidget *parent /*= 0*/) : QDialog(parent), _tabWidget(new QTabWidget(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    setWindowTitle(tr("Skill tree"));

    const CharacterInfo::CharacterInfoBasic &charInfo = CharacterInfo::instance().basicInfo;
    qint32 addSkillPoints = 0;
    ItemsList itemsWithBonuses;
    QMultiHash<QByteArray, int> setItemsHash;

    foreach (ItemInfo *item, CharacterInfo::instance().items.character) //-V807
    {
        if (ItemDataBase::doesItemGrantBonus(item))
        {
            itemsWithBonuses << item;
            addSkillPoints += getValueOfPropertyInItem(item, Enums::ItemProperties::AllSkills);
            addSkillPoints += getValueOfPropertyInItem(item, Enums::ItemProperties::ClassSkills, charInfo.classCode);

            if (item->quality == Enums::ItemQuality::Set)
                setItemsHash.insert(ItemDataBase::Sets()->value(item->setOrUniqueId)->key, item->setOrUniqueId);
        }
    }

    foreach (const QByteArray &setKey, setItemsHash.uniqueKeys())
    {
        QList<int> setItemIds = setItemsHash.values(setKey);
        if (quint8 partialPropsNumber = (setItemIds.size() - 1) * 2)
        {
            foreach (int setId, setItemIds)
            {
                const QList<SetFixedProperty> &setProps = ItemDataBase::Sets()->value(setId)->fixedProperties;
                addSkillPoints += getValueOfPropertyInSetProperties(setProps, Enums::ItemProperties::AllSkills,   partialPropsNumber);
                addSkillPoints += getValueOfPropertyInSetProperties(setProps, Enums::ItemProperties::ClassSkills, partialPropsNumber, charInfo.classCode);
            }

            const FullSetInfo fullSetInfo = ItemDataBase::fullSetInfoForKey(setKey);
            addSkillPoints += getValueOfPropertyInSetProperties(fullSetInfo.partialSetProperties, Enums::ItemProperties::AllSkills,   partialPropsNumber);
            addSkillPoints += getValueOfPropertyInSetProperties(fullSetInfo.partialSetProperties, Enums::ItemProperties::ClassSkills, partialPropsNumber, charInfo.classCode);

            if (setItemIds.size() == fullSetInfo.itemNames.size())
            {
                addSkillPoints += getValueOfPropertyInSetProperties(fullSetInfo.fullSetProperties, Enums::ItemProperties::AllSkills,   partialPropsNumber);
                addSkillPoints += getValueOfPropertyInSetProperties(fullSetInfo.fullSetProperties, Enums::ItemProperties::ClassSkills, partialPropsNumber, charInfo.classCode);
            }
        }
    }

    const QList<int> skillsVisualOrder = Enums::Skills::currentCharacterSkillsIndexes().second;
    for (int i = 0, j = 0, skillsNumber = skillsVisualOrder.size(); i < 3; ++i)
    {
        int tabIndex = i + 1;
        QWidget *tab = new QWidget(this);
        _tabWidget->addTab(tab, tr("Tab %1").arg(tabIndex));

        QGridLayout *grid = new QGridLayout(tab);
        grid->setContentsMargins(QMargins());

        QGroupBox *uberSkillsBox = 0;
        while (j < skillsNumber)
        {
            int skillIndex = skillsVisualOrder.at(j);
            SkillInfo *skill = ItemDataBase::Skills()->value(skillIndex);
            if (skill->tab != tabIndex)
                break;

            qint32 baseSkillPoints = charInfo.skillsReadable.at(j), totalSkillPoints = baseSkillPoints ? addSkillPoints : 0;
            foreach (ItemInfo *item, itemsWithBonuses)
            {
                totalSkillPoints += getValueOfPropertyInItem(item, Enums::ItemProperties::ClassOnlySkill, skillIndex);
                totalSkillPoints += qMin(3, getValueOfPropertyInItem(item, Enums::ItemProperties::Oskill, skillIndex));
            }

            SkillWidget *w = new SkillWidget(tab);
            w->setSkillName(skill->name);
            w->setSkillImageForClassWithId(charInfo.classCode, skill->imageId);
            w->setSkillPoints(baseSkillPoints, totalSkillPoints);

            if (skillsNumber == 30 && i == 2 && skill->col == 3) // Ultimative XV has 35 skills and different uberskills tab layout
            {
                if (!uberSkillsBox)
                {
                    uberSkillsBox = new QGroupBox(tr("%1berskills", "param is letter U with umlaut").arg(QChar(0x00dc)), tab);
                    uberSkillsBox->setLayout(new QVBoxLayout);
                    uberSkillsBox->layout()->setContentsMargins(QMargins());
                    grid->addWidget(uberSkillsBox, 0, 1, 5, 1);
                }
                uberSkillsBox->layout()->addWidget(w);
            }
            else
                grid->addWidget(w, skill->row - 1, skill->col - 1);
            ++j;
        }
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addWidget(_tabWidget);
    layout->addWidget(buttonBox);

    QVariant savedPos = QSettings().value("skillTreePos");
    if (savedPos.isValid())
        move(savedPos.toPoint());
}

void SkillTreeDialog::reject()
{
    QSettings().setValue("skillTreePos", pos());
    QDialog::reject();
}

qint32 SkillTreeDialog::getValueOfPropertyInSetProperties(const QList<SetFixedProperty> &setProps, quint16 propKey, quint8 propsNumber /*= 0*/, quint16 param /*= 0*/)
{
    qint32 result = 0;
    for (quint8 i = 0, n = propsNumber ? qMin(propsNumber, static_cast<quint8>(setProps.size())) : setProps.size(); i < n; ++i)
    {
        const SetFixedProperty &setProp = setProps.at(i);
        if (setProp.ids.indexOf(propKey) != -1 && setProp.param == param)
            result += setProp.maxValue;
    }
    return result;
}
