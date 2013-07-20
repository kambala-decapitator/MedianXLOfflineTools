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


class SkillWidget : public QWidget
{
public:
    SkillWidget(QWidget *parent = 0) : QWidget(parent), _skillNameLabel(new QLabel(this)), _skillImageLabel(new QLabel(this)), _skillPointsLabel(new QLabel(this))
    {
        QGridLayout *layout = new QGridLayout(this);
        layout->addWidget(_skillNameLabel, 0, 0);
        layout->addWidget(_skillImageLabel, 1, 0, Qt::AlignHCenter);
        layout->addWidget(_skillPointsLabel, 2, 0, Qt::AlignHCenter);
    }
    virtual ~SkillWidget() {}

    void setSkillName(const QString &name) { _skillNameLabel->setText(name); _skillNameLabel->adjustSize(); }
    void setSkillImage(const QString &imagePath) { _skillImageLabel->setPixmap(QPixmap(imagePath)); }
    void setSkillPoints(quint8 basePoints, quint8 addPoints)
    {
        _skillPointsLabel->setText(addPoints ? QString("%1 (%2)").arg(basePoints + addPoints).arg(basePoints) : QString::number(basePoints));
        if (!basePoints)
            setDisabled(true);
    }

private:
    QLabel *_skillNameLabel, *_skillImageLabel, *_skillPointsLabel;
};


SkillTreeDialog::SkillTreeDialog(const SkillsOrderPair &skillsOrderPair, QWidget *parent) : QDialog(parent), _tabWidget(new QTabWidget(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    setWindowTitle(tr("Skill tree"));

    const CharacterInfo::CharacterInfoBasic &charInfo = CharacterInfo::instance().basicInfo;
    qint32 addSkillPoints = 0;
    ItemsList itemsWithBonuses;
    foreach (ItemInfo *item, CharacterInfo::instance().items.character)
    {
        if (ItemDataBase::doesItemGrantBonus(item))
        {
            itemsWithBonuses << item;
            addSkillPoints += getValueOfPropertyInItem(item, Enums::ItemProperties::AllSkills);
            addSkillPoints += getValueOfPropertyInItem(item, Enums::ItemProperties::ClassSkills, charInfo.classCode);
        }
    }

    for (int i = 0, j = 0; i < 3; ++i)
    {
        int tabIndex = i + 1;
        QWidget *tab = new QWidget(this);
        _tabWidget->addTab(tab, tr("Skill Tab %1").arg(tabIndex));

        QGroupBox *uberSkillsBox = 0;
        QGridLayout *grid = new QGridLayout(tab);
        grid->setContentsMargins(QMargins());
        while (j < skillsOrderPair.second.size())
        {
            int skillIndex = skillsOrderPair.second.at(j);
            SkillInfo *skill = ItemDataBase::Skills()->value(skillIndex);
            if (skill->tab != tabIndex)
                break;

            // TODO: test calculation correctness
            qint32 baseSkillPoints = charInfo.skillsReadable.at(j), totalSkillPoints = baseSkillPoints ? addSkillPoints : 0;
            foreach (ItemInfo *item, itemsWithBonuses)
            {
                totalSkillPoints += getValueOfPropertyInItem(item, Enums::ItemProperties::ClassOnlySkill, skillIndex);
                totalSkillPoints += qMin(3, getValueOfPropertyInItem(item, Enums::ItemProperties::Oskill, skillIndex)); // TODO: check minimum
            }

            SkillWidget *w = new SkillWidget(tab);
            w->setSkillName(skill->name);
            w->setSkillImage(ResourcePathManager::pathForSkillImage(charInfo.classCode, skill->imageId));
            w->setSkillPoints(baseSkillPoints, totalSkillPoints);

            if (i == 2 && skill->col == 3) // no idea why uberskills have column 3
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
}
