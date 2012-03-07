#ifndef SKILLPLANDIALOG_H
#define SKILLPLANDIALOG_H

#include "ui_skillplandialog.h"

#include <QDialog>


class SkillplanDialog : public QDialog
{
    Q_OBJECT

public:
    SkillplanDialog(QWidget *parent = 0);

    static void loadModVersion();
    static const QString &modVersionReadable() { return _modVersionReadable; }
    static const QString &modVersionPlanner()  { return _modVersionPlanner;  }

private slots:
    void copyHtml();
    void copyBbcode();

private:
    Ui::SkillplanDialog ui;

    static QString _modVersionReadable, _modVersionPlanner;
};

#endif // SKILLPLANDIALOG_H
