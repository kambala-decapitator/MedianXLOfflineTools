#ifndef SKILLPLANDIALOG_H
#define SKILLPLANDIALOG_H

#include "ui_skillplandialog.h"

#include <QDialog>


class ItemInfo;
class CharacterInfo;

class SkillplanDialog : public QDialog
{
    Q_OBJECT

public:
    SkillplanDialog(QWidget *parent = 0);
    virtual ~SkillplanDialog() {}

    static bool didModVersionChange();
    static const QString &modVersionReadable() { return _modVersionReadable; }
    static const QString &modVersionPlanner()  { return _modVersionPlanner;  }

private slots:
    void copyLinkWithTags();
    void constructUrls();
    void showHelp();

    virtual void done(int r);

private:
    Ui::SkillplanDialog ui;

    static QString _modVersionReadable, _modVersionPlanner;

    void loadSettings();
    void saveSettings();

    ItemInfo *itemOr0(const CharacterInfo &charInfo, bool(*predicate)(ItemInfo *));
    QString hasItemBoolString(const CharacterInfo &charInfo, bool(*predicate)(ItemInfo *)) { return QString::number(itemOr0(charInfo, predicate) != 0); }
};

#endif // SKILLPLANDIALOG_H
