#ifndef EXPERIENCEINDICATORGROUPBOX_H
#define EXPERIENCEINDICATORGROUPBOX_H

#include <QGroupBox>
#include <QProgressBar>


class ExperienceIndicatorGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    ExperienceIndicatorGroupBox(QWidget *parent);

    void setCurrentExperience(quint32 exp)   { _progressBar->setValue(exp); }
    void setNextLevelExperience(quint32 exp) { _progressBar->setMaximum(exp); }

private:
    QProgressBar *_progressBar;
};

#endif // EXPERIENCEINDICATORGROUPBOX_H
