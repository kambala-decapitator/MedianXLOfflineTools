#include "experienceindicatorgroupbox.h"

#include <QProgressBar>
#include <QHBoxLayout>


ExperienceIndicatorGroupBox::ExperienceIndicatorGroupBox(QWidget *parent) : QGroupBox(tr("Experience"), parent), _progressBar(new QProgressBar(this))
{
    _progressBar->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    _progressBar->setFormat("%v / %m (%p%)");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(_progressBar);
}

void ExperienceIndicatorGroupBox::setNextLevelExperience(quint32 exp)
{
    _progressBar->setMaximum(exp);
}

void ExperienceIndicatorGroupBox::setCurrentExperience(quint32 exp)
{
    _progressBar->setValue(exp);
    // Mac OS X doesn't display text on the progressbar, so let's display it in the statusbar
#ifdef Q_WS_MACX
    _progressBar->setStatusTip(_progressBar->text());
#endif
}
