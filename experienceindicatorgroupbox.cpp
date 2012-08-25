#include "experienceindicatorgroupbox.h"

#include <QProgressBar>
#include <QHBoxLayout>


ExperienceIndicatorGroupBox::ExperienceIndicatorGroupBox(QWidget *parent) : QGroupBox(tr("Experience"), parent), _progressBar(new QProgressBar(this))
{
    _progressBar->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(_progressBar);
}

void ExperienceIndicatorGroupBox::setPreviousLevelExperience(quint32 exp)
{
    _progressBar->setMinimum(exp);
}

void ExperienceIndicatorGroupBox::setNextLevelExperience(quint32 exp)
{
    _progressBar->setMaximum(exp);
}

void ExperienceIndicatorGroupBox::setCurrentExperience(quint32 exp)
{
    _progressBar->setValue(exp);
    _progressBar->setFormat(QString("%v / %1 (%p%)").arg(_progressBar->maximum()));
    // Mac OS X doesn't display text on the progressbar, so let's display it in the statusbar
#ifdef Q_WS_MACX
    _progressBar->setStatusTip(_progressBar->text());
#endif
}

void ExperienceIndicatorGroupBox::reset()
{
    _progressBar->reset();
#ifdef Q_WS_MACX
    _progressBar->setStatusTip(QString());
#endif
}
