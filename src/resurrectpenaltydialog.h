#ifndef RESURRECTPENALTYDIALOG_H
#define RESURRECTPENALTYDIALOG_H

#include <QDialog>


namespace Ui { class ResurrectPenaltyDialog; }

class ResurrectPenaltyDialog : public QDialog
{
    Q_OBJECT

public:
    ResurrectPenaltyDialog(QWidget *parent = 0);
    virtual ~ResurrectPenaltyDialog();

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
    Ui::ResurrectPenaltyDialog *ui;
};

#endif // RESURRECTPENALTYDIALOG_H
