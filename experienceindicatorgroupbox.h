#ifndef EXPERIENCEINDICATORGROUPBOX_H
#define EXPERIENCEINDICATORGROUPBOX_H

#include <QGroupBox>


class QProgressBar;

class ExperienceIndicatorGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    ExperienceIndicatorGroupBox(QWidget *parent);
    virtual ~ExperienceIndicatorGroupBox() {}

    void setPreviousLevelExperience(quint32 exp);
    void setNextLevelExperience(quint32 exp);
    void setCurrentExperience(quint32 exp);
    void reset();

private:
    QProgressBar *_progressBar;
};

#endif // EXPERIENCEINDICATORGROUPBOX_H
