#ifndef RESURRECTPENALTYDIALOG_H
#define RESURRECTPENALTYDIALOG_H

#include "ui_resurrectpenaltydialog.h"

#include <QDialog>


class ResurrectPenaltyDialog : public QDialog
{
    Q_OBJECT

public:
    ResurrectPenaltyDialog(QWidget *parent = 0);

    static const int kLevelPenalty;
    static const double kStatPenalty, kSkillPenalty;

    enum ResurrectionPenalty
    {
        Nothing = -1,
        Levels,
        Skills,
        Stats
    };

    ResurrectionPenalty resurrectionPenalty() const;

private:
    Ui::ResurrectPenaltyDialog ui;
};

#endif // RESURRECTPENALTYDIALOG_H
